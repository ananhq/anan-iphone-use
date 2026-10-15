// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdbool.h>
#include <stdint.h>
#define WIFI_QR_MAX_MODULES 33
#define WIFI_QR_BYTES ((WIFI_QR_MAX_MODULES*WIFI_QR_MAX_MODULES+7)/8+1)
typedef struct {uint8_t bytes[WIFI_QR_BYTES];} wifi_qr;
bool wifi_qr_make(wifi_qr *out,const char *ssid,const char *password);
unsigned wifi_qr_size(const wifi_qr *code);
bool wifi_qr_module(const wifi_qr *code,int x,int y);
