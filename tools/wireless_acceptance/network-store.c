// SPDX-License-Identifier: Apache-2.0
// Scenarios defined before implementation. No SDK, network or device access.
#include "network_store.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    network_store s={0};
    network_legacy old={.magic=NETWORK_LEGACY_MAGIC,.ssid="Old",.password="password"};
    assert(network_store_decode(&s,&old,sizeof(old)) && s.count==1);
    assert(network_store_put(&s,"Second","password2"));
    assert(s.count==2 && !strcmp(s.entries[0].ssid,"Second"));
    assert(network_store_put(&s,"Old","changed-pass") && s.count==2);
    assert(!strcmp(s.entries[0].password,"changed-pass"));
    assert(network_store_put(&s,"3","password"));
    assert(network_store_put(&s,"4","password"));
    assert(network_store_put(&s,"5","password"));
    assert(!network_store_put(&s,"6","password") && s.count==5);
    assert(network_store_remove(&s,"Second") && s.count==4);
    assert(network_store_put(&s,"6","password"));
    network_store copy;
    assert(network_store_decode(&copy,&s,sizeof(s)) && copy.count==5);
    s.count=6; assert(!network_store_decode(&copy,&s,sizeof(s)));
    memset(old.ssid,'x',sizeof(old.ssid));
    assert(!network_store_decode(&copy,&old,sizeof(old)));
    assert(!network_store_put(&copy,"Test","short"));
    assert(!network_store_put(&copy,"","password"));
    puts("Network migration, update order, capacity, removal and invalid storage passed (offline).");
}
