// SPDX-License-Identifier: Apache-2.0
// Pre-implementation HTTP authority scenarios. No sockets or device access.
#include "network_authority.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
 assert(network_authority("stopwatch.local","http://stopwatch.local","192.168.7.1",false,true));
 assert(network_authority("stopwatch.local:80","http://stopwatch.local:80","10.0.0.2",false,true));
 assert(network_authority("192.168.4.1",NULL,"192.168.4.1",false,true));
 assert(!network_authority("evil.local",NULL,"192.168.4.1",false,true));
 assert(!network_authority("stopwatch.local","http://evil.local","10.0.0.2",false,true));
 assert(!network_authority("stopwatch.local","http://10.0.0.2","10.0.0.2",false,true));
 assert(!network_authority("stopwatch.local",NULL,"10.0.0.2",false,false));
 assert(network_authority("192.168.7.1:8443","https://192.168.7.1:8443","192.168.7.1",true,false));
 assert(!network_authority("stopwatch.local:8443",NULL,"192.168.7.1",true,true));
 puts("HTTP authority and exact-origin cases passed (offline).");
}
