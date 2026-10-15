// SPDX-License-Identifier: Apache-2.0
#include "wifi_qr.h"
#include "vendor/qrcodegen.h"
#include <string.h>
static void clear(void *data,size_t size) {
    volatile unsigned char *p=data;
    while(size--)*p++=0;
}
static bool append(char *out,size_t *at,size_t size,const char *value,bool escape) {
    for(;*value;value++) {
        unsigned char c=*value;
        if(c<32||c==127)return false;
        if(escape&&strchr("\\;,:\"",c)) {
            if(*at+1>=size)return false;
            out[(*at)++]='\\';
        }
        if(*at+1>=size)return false;
        out[(*at)++]=c;
    }
    out[*at]=0;
    return true;
}
bool wifi_qr_make(wifi_qr *out,const char *ssid,const char *password) {
    memset(out,0,sizeof(*out));
    size_t name_len=strnlen(ssid,33),pass_len=strnlen(password,64);
    if(!name_len||name_len>32||pass_len<8||pass_len>63)return false;
    char text[224]={0};size_t at=0;
    uint8_t work[WIFI_QR_BYTES]={0};
    bool valid=append(text,&at,sizeof(text),"WIFI:T:WPA;S:",false)&&
               append(text,&at,sizeof(text),ssid,true)&&
               append(text,&at,sizeof(text),";P:",false)&&
               append(text,&at,sizeof(text),password,true)&&
               append(text,&at,sizeof(text),";;",false);
    bool ok=valid&&qrcodegen_encodeText(text,work,out->bytes,qrcodegen_Ecc_MEDIUM,
                                       1,4,qrcodegen_Mask_AUTO,true);
    clear(text,sizeof(text));clear(work,sizeof(work));
    if(!ok)memset(out,0,sizeof(*out));
    return ok;
}
unsigned wifi_qr_size(const wifi_qr *code) {return code->bytes[0];}
bool wifi_qr_module(const wifi_qr *code,int x,int y) {
    unsigned side=wifi_qr_size(code);
    return side&&x>=0&&y>=0&&(unsigned)x<side&&(unsigned)y<side&&qrcodegen_getModule(code->bytes,x,y);
}
