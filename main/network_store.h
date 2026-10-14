// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define NETWORK_LIMIT 5
#define NETWORK_LEGACY_MAGIC 0x53574e31u
#define NETWORK_STORE_MAGIC 0x53574e32u
typedef struct { char ssid[33], password[65]; } network_entry;
typedef struct { uint32_t magic; char ssid[33], password[65]; } network_legacy;
typedef struct { uint32_t magic, count; network_entry entries[NETWORK_LIMIT]; } network_store;
bool network_store_decode(network_store *out,const void *data,size_t size);
bool network_store_put(network_store *s,const char *ssid,const char *password);
bool network_store_remove(network_store *s,const char *ssid);
