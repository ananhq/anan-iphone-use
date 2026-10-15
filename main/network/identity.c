// SPDX-License-Identifier: Apache-2.0
#include "identity.h"
#include "bootloader_random.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecp.h"
#include "mbedtls/oid.h"
#include "mbedtls/pk.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/sha256.h"
#include "mbedtls/x509_crt.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdatomic.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IDENTITY_MAGIC 0x53574d31u
typedef struct {
    uint32_t magic;
    char ca_key[512], key[512], cert[2048];
    unsigned char ca_der[1024];
    uint32_t ca_der_length;
    char token[65], fingerprint[65];
    int64_t expires_at;
} stored_identity;
static stored_identity stored;
static atomic_bool ready;
static mbedtls_ctr_drbg_context rng;
static SemaphoreHandle_t rng_lock;
static nvs_handle_t storage;
static bool storage_ok;

static int entropy(void *context, unsigned char *out, size_t len) {
    (void)context;
    esp_fill_random(out, len);
    return 0;
}
static int random_bytes(void *context, unsigned char *out, size_t len) {
    (void)context;
    xSemaphoreTake(rng_lock, portMAX_DELAY);
    int result = mbedtls_ctr_drbg_random(&rng, out, len);
    xSemaphoreGive(rng_lock);
    return result;
}
bool identity_random_hex(char *out, size_t bytes) {
    if (bytes == 0 || bytes > 32)
        return false;
    unsigned char data[32];
    if (random_bytes(NULL, data, bytes) != 0)
        return false;
    static const char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < bytes; ++i) {
        out[2 * i] = hex[data[i] >> 4];
        out[2 * i + 1] = hex[data[i] & 15];
    }
    out[2 * bytes] = 0;
    mbedtls_platform_zeroize(data, sizeof(data));
    return true;
}
void identity_init(void) {
    rng_lock = xSemaphoreCreateMutex();
    configASSERT(rng_lock);
    mbedtls_ctr_drbg_init(&rng);
    // Seed once from the documented non-RF entropy source before any ADC/radio use.
    bootloader_random_enable();
    int seeded =
        mbedtls_ctr_drbg_seed(&rng, entropy, NULL, (const unsigned char *)"stopwatch-mcp", 13);
    bootloader_random_disable();
    configASSERT(seeded == 0);
    // Keep the seeded DRBG state; do not auto-reseed from the disabled ADC source.
    mbedtls_ctr_drbg_set_reseed_interval(&rng, INT_MAX);
    if (nvs_flash_init_partition("mcp_kv") != ESP_OK ||
        nvs_open_from_partition("mcp_kv", "identity", NVS_READWRITE, &storage) != ESP_OK)
        return;
    storage_ok = true;
    size_t size = sizeof(stored);
    if (nvs_get_blob(storage, "v1", &stored, &size) == ESP_OK && size == sizeof(stored) &&
        stored.magic == IDENTITY_MAGIC && stored.ca_der_length > 0 &&
        stored.ca_der_length <= sizeof(stored.ca_der) &&
        memchr(stored.ca_key, 0, sizeof(stored.ca_key)) &&
        memchr(stored.key, 0, sizeof(stored.key)) && memchr(stored.cert, 0, sizeof(stored.cert)) &&
        stored.token[64] == 0 && strlen(stored.token) == 64 && stored.fingerprint[64] == 0) {
        atomic_store(&ready, true);
    } else {
        mbedtls_platform_zeroize(&stored, sizeof(stored));
    }
}
bool identity_ready(void) { return atomic_load(&ready); }
const char *identity_certificate(void) { return stored.cert; }
const char *identity_private_key(void) { return stored.key; }
const char *identity_token(void) { return stored.token; }
const char *identity_fingerprint(void) { return stored.fingerprint; }
const unsigned char *identity_ca_der(size_t *length) {
    *length = stored.ca_der_length;
    return stored.ca_der;
}
static int new_key(mbedtls_pk_context *key) {
    int result = mbedtls_pk_setup(key, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if (result == 0)
        result =
            mbedtls_ecp_gen_key(MBEDTLS_ECP_DP_SECP256R1, mbedtls_pk_ec(*key), random_bytes, NULL);
    return result;
}
static int certificate(mbedtls_x509write_cert *crt, mbedtls_pk_context *subject,
                       mbedtls_pk_context *issuer, bool ca, time_t now) {
    const char *issuer_name = "CN=StopWatch Local CA,O=StopWatch";
    unsigned char serial[16];
    int result = random_bytes(NULL, serial, sizeof(serial));
    serial[0] &= 0x7f;
    serial[0] |= 1;
    if (result != 0)
        return result;
    mbedtls_x509write_crt_set_version(crt, MBEDTLS_X509_CRT_VERSION_3);
    mbedtls_x509write_crt_set_md_alg(crt, MBEDTLS_MD_SHA256);
    mbedtls_x509write_crt_set_subject_key(crt, subject);
    mbedtls_x509write_crt_set_issuer_key(crt, issuer);
    if ((result = mbedtls_x509write_crt_set_serial_raw(crt, serial, sizeof(serial))) != 0 ||
        (result = mbedtls_x509write_crt_set_issuer_name(crt, issuer_name)) != 0 ||
        (result = mbedtls_x509write_crt_set_subject_name(
             crt, ca ? issuer_name : "CN=StopWatch,O=StopWatch")) != 0 ||
        (result = mbedtls_x509write_crt_set_basic_constraints(crt, ca, ca ? 0 : -1)) != 0 ||
        (result = mbedtls_x509write_crt_set_key_usage(
             crt, ca ? MBEDTLS_X509_KU_KEY_CERT_SIGN | MBEDTLS_X509_KU_CRL_SIGN
                     : MBEDTLS_X509_KU_DIGITAL_SIGNATURE)) != 0 ||
        (result = mbedtls_x509write_crt_set_subject_key_identifier(crt)) != 0 ||
        (result = mbedtls_x509write_crt_set_authority_key_identifier(crt)) != 0)
        return result;
    char before[16], after[16];
    struct tm date;
    time_t start = now - 86400;
    time_t finish = now + (time_t)(ca ? 7300 : 365) * 86400;
    gmtime_r(&start, &date);
    strftime(before, sizeof(before), "%Y%m%d%H%M%S", &date);
    gmtime_r(&finish, &date);
    strftime(after, sizeof(after), "%Y%m%d%H%M%S", &date);
    if ((result = mbedtls_x509write_crt_set_validity(crt, before, after)) != 0)
        return result;
    if (ca) {
        // Critical constraints restrict this CA to the device IP and its local DNS name.
        static const unsigned char permitted[] = {
            0x30, 0x21, 0xa0, 0x1f, 0x30, 0x0a, 0x87, 0x08, 192, 168, 7,   1,
            255,  255,  255,  255,  0x30, 0x11, 0x82, 0x0f, 's', 't', 'o', 'p',
            'w',  'a',  't',  'c',  'h',  '.',  'l',  'o',  'c', 'a', 'l'};
        result = mbedtls_x509write_crt_set_extension(crt, MBEDTLS_OID_NAME_CONSTRAINTS,
                                                     MBEDTLS_OID_SIZE(MBEDTLS_OID_NAME_CONSTRAINTS),
                                                     1, permitted, sizeof(permitted));
    } else {
        unsigned char ip[] = {192, 168, 7, 1};
        mbedtls_x509_san_list dns = {0}, address = {0};
        dns.node.type = MBEDTLS_X509_SAN_DNS_NAME;
        dns.node.san.unstructured_name.p = (unsigned char *)"stopwatch.local";
        dns.node.san.unstructured_name.len = 15;
        dns.next = &address;
        address.node.type = MBEDTLS_X509_SAN_IP_ADDRESS;
        address.node.san.unstructured_name.p = ip;
        address.node.san.unstructured_name.len = sizeof(ip);
        result = mbedtls_x509write_crt_set_subject_alternative_name(crt, &dns);
        static const unsigned char eku[] = {0x30, 0x0a, 0x06, 0x08, 0x2b, 0x06,
                                            0x01, 0x05, 0x05, 0x07, 0x03, 0x01};
        if (result == 0)
            result = mbedtls_x509write_crt_set_extension(
                crt, MBEDTLS_OID_EXTENDED_KEY_USAGE,
                MBEDTLS_OID_SIZE(MBEDTLS_OID_EXTENDED_KEY_USAGE), 0, eku, sizeof(eku));
    }
    return result;
}
esp_err_t identity_prepare(time_t now) {
    if (!storage_ok || now < 1735689600 || now > 4102444800LL)
        return ESP_ERR_INVALID_STATE;
    if (identity_ready() && stored.expires_at > (int64_t)now + 7 * 86400)
        return ESP_OK;
    stored_identity *candidate = calloc(1, sizeof(*candidate));
    if (!candidate)
        return ESP_ERR_NO_MEM;
    if (identity_ready())
        memcpy(candidate, &stored, sizeof(*candidate));
    mbedtls_pk_context ca, key;
    mbedtls_pk_init(&ca);
    mbedtls_pk_init(&key);
    mbedtls_x509write_cert root, leaf;
    mbedtls_x509write_crt_init(&root);
    mbedtls_x509write_crt_init(&leaf);
    bool existing = identity_ready();
    int result =
        existing ? mbedtls_pk_parse_key(&ca, (unsigned char *)candidate->ca_key,
                                        strlen(candidate->ca_key) + 1, NULL, 0, random_bytes, NULL)
                 : new_key(&ca);
    if (result == 0)
        result = new_key(&key);
    if (result == 0 && !existing)
        result = certificate(&root, &ca, &ca, true, now);
    if (result == 0 && !existing) {
        unsigned char der[1024];
        int len = mbedtls_x509write_crt_der(&root, der, sizeof(der), random_bytes, NULL);
        if (len < 0)
            result = len;
        else {
            candidate->ca_der_length = len;
            memcpy(candidate->ca_der, der + sizeof(der) - len, len);
            unsigned char digest[32];
            result = mbedtls_sha256(candidate->ca_der, len, digest, 0);
            for (int i = 0; result == 0 && i < 32; ++i)
                snprintf(candidate->fingerprint + 2 * i, 3, "%02x", digest[i]);
        }
    }
    if (result == 0)
        result = certificate(&leaf, &key, &ca, false, now);
    if (result == 0)
        result = mbedtls_x509write_crt_pem(&leaf, (unsigned char *)candidate->cert,
                                           sizeof(candidate->cert), random_bytes, NULL);
    if (result == 0)
        result =
            mbedtls_pk_write_key_pem(&key, (unsigned char *)candidate->key, sizeof(candidate->key));
    if (result == 0 && !existing)
        result = mbedtls_pk_write_key_pem(&ca, (unsigned char *)candidate->ca_key,
                                          sizeof(candidate->ca_key));
    if (result == 0 && !existing && !identity_random_hex(candidate->token, 32))
        result = -1;
    if (result == 0) {
        candidate->magic = IDENTITY_MAGIC;
        candidate->expires_at = (int64_t)now + 365 * 86400;
        esp_err_t saved = nvs_set_blob(storage, "v1", candidate, sizeof(*candidate));
        if (saved == ESP_OK)
            saved = nvs_commit(storage);
        if (saved == ESP_OK) {
            memcpy(&stored, candidate, sizeof(stored));
            atomic_store(&ready, true);
        } else
            result = -1;
    }
    mbedtls_x509write_crt_free(&root);
    mbedtls_x509write_crt_free(&leaf);
    mbedtls_pk_free(&ca);
    mbedtls_pk_free(&key);
    mbedtls_platform_zeroize(candidate, sizeof(*candidate));
    free(candidate);
    return result == 0 ? ESP_OK : ESP_FAIL;
}
esp_err_t identity_reset(void) {
    if (!storage_ok)
        return ESP_ERR_INVALID_STATE;
    esp_err_t result = nvs_erase_all(storage);
    if (result == ESP_OK)
        result = nvs_commit(storage);
    if (result == ESP_OK) {
        atomic_store(&ready, false);
        mbedtls_platform_zeroize(&stored, sizeof(stored));
    }
    return result;
}
