// SPDX-License-Identifier: Apache-2.0
#include "network_store.h"
#include <string.h>
static bool valid(const char *ssid,const char *password) {
 size_t n=strnlen(ssid,33),p=strnlen(password,65);
 if (!n || n>32 || p>64 || (p && p<8)) return false;
 if (p==64) for(unsigned i=0;i<64;i++)
  if(!((password[i]>='0'&&password[i]<='9')||(password[i]>='a'&&password[i]<='f')||(password[i]>='A'&&password[i]<='F')))return false;
 return true;
}
bool network_store_decode(network_store *out,const void *data,size_t size) {
 memset(out,0,sizeof(*out));out->magic=NETWORK_STORE_MAGIC;
 if(size==sizeof(network_legacy)) {
  network_legacy old;memcpy(&old,data,sizeof(old));
  if(old.magic!=NETWORK_LEGACY_MAGIC || !memchr(old.ssid,0,33) || !memchr(old.password,0,65))return false;
  if(!old.ssid[0])return !old.password[0];
  return network_store_put(out,old.ssid,old.password);
 }
 if(size!=sizeof(*out))return false;
 network_store candidate;memcpy(&candidate,data,sizeof(candidate));
 if(candidate.magic!=NETWORK_STORE_MAGIC || candidate.count>NETWORK_LIMIT)return false;
 for(unsigned i=0;i<candidate.count;i++) {
  if(!valid(candidate.entries[i].ssid,candidate.entries[i].password))return false;
  for(unsigned j=0;j<i;j++)if(!strcmp(candidate.entries[i].ssid,candidate.entries[j].ssid))return false;
 }
 *out=candidate;return true;
}
bool network_store_put(network_store *s,const char *ssid,const char *password) {
 if(!valid(ssid,password) || s->count>NETWORK_LIMIT)return false;
 unsigned at=s->count;
 for(unsigned i=0;i<s->count;i++)if(!strcmp(s->entries[i].ssid,ssid)){at=i;break;}
 if(at==s->count && s->count==NETWORK_LIMIT)return false;
 network_entry entry={0};strcpy(entry.ssid,ssid);strcpy(entry.password,password);
 if(at==s->count)++s->count;
 memmove(&s->entries[1],&s->entries[0],at*sizeof(entry));s->entries[0]=entry;
 s->magic=NETWORK_STORE_MAGIC;return true;
}
bool network_store_remove(network_store *s,const char *ssid) {
 for(unsigned i=0;i<s->count;i++)if(!strcmp(s->entries[i].ssid,ssid)) {
  memmove(&s->entries[i],&s->entries[i+1],(s->count-i-1)*sizeof(network_entry));
  memset(&s->entries[--s->count],0,sizeof(network_entry));return true;
 }
 return false;
}
