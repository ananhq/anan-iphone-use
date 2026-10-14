// SPDX-License-Identifier: Apache-2.0
// Acceptance defined before QR integration. Synthetic credentials only.
#include "wifi_qr.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv) {
    assert(argc==2);
    wifi_qr code;
    assert(wifi_qr_make(&code,"StopWatch-setup","DEMO23456789"));
    assert(wifi_qr_size(&code)>0 && wifi_qr_size(&code)<=WIFI_QR_MAX_MODULES);
    FILE *out=fopen(argv[1],"wb");assert(out);
    unsigned side=(wifi_qr_size(&code)+8)*5;
    fprintf(out,"P5\n%u %u\n255\n",side,side);
    for(unsigned y=0;y<side;y++)for(unsigned x=0;x<side;x++) {
        int cx=(int)x/5-4,cy=(int)y/5-4;
        fputc(wifi_qr_module(&code,cx,cy)?0:255,out);
    }
    fclose(out);
    assert(!wifi_qr_make(&code,"","DEMO23456789"));
    assert(!wifi_qr_size(&code));
    assert(!wifi_qr_make(&code,"StopWatch-setup","short"));
    assert(!wifi_qr_make(&code,"name-with-newline\n","DEMO23456789"));
    assert(wifi_qr_make(&code,"semi;colon","DEMO23456789"));
    puts("Wi-Fi QR generation, invalid input clearing and escaped SSID cases passed (synthetic).");
}
