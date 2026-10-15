// SPDX-License-Identifier: Apache-2.0
#include "network_authority.h"
#include <stdio.h>
#include <string.h>
bool network_authority(const char *host,const char *origin,const char *address,bool secure,bool hostname) {
 if(!host || !address)return false;
 char authority[64];snprintf(authority,sizeof(authority),secure?"%s:8443":"%s:80",address);
 bool valid=!strcmp(host,authority)||(!secure&&!strcmp(host,address));
 if(!secure&&hostname)valid=valid||!strcmp(host,"stopwatch.local")||!strcmp(host,"stopwatch.local:80");
 if(!valid)return false;
 if(!origin || !origin[0])return true;
 char expected[80];snprintf(expected,sizeof(expected),"%s://%s",secure?"https":"http",host);
 return !strcmp(origin,expected);
}
