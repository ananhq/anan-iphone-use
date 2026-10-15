// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include "esp_err.h"

void identity_init(void);
bool identity_ready(void);
esp_err_t identity_prepare(time_t now);
esp_err_t identity_reset(void);
bool identity_random_hex(char *out, size_t bytes);
const char *identity_certificate(void);
const char *identity_private_key(void);
const char *identity_token(void);
const char *identity_fingerprint(void);
const unsigned char *identity_ca_der(size_t *length);
