#!/usr/bin/env python3
# First-party regression scenario defined before the mDNS probe correction.
# Runs the pinned SDK timer function with fake sends/timers, no actual sockets.
import os,pathlib,subprocess,sys,tempfile
source=pathlib.Path(sys.argv[1]).read_text()
start=source.index('static void\nmdns_probe_and_announce(void* arg)')
body=source[start:source.index('\n}\n',start)+3]
prelude=r'''
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
typedef uint32_t u32_t;
#define LWIP_IPV4 1
#define LWIP_IPV6 1
#define ERR_OK 0
#define MDNS_PROBE_COUNT 3
#define MDNS_ANNOUNCE_COUNT 2
#define MDNS_PROBE_MAX_CONFLICTS_TIMEOUT 5000
#define MDNS_PROBE_DELAY_MS 250
#define MDNS_ANNOUNCE_DELAY_MS 1000
#define MDNS_PROBING_SUCCESSFUL 1
enum { MDNS_STATE_OFF,MDNS_STATE_PROBE_WAIT,MDNS_STATE_PROBING,MDNS_STATE_ANNOUNCE_WAIT,MDNS_STATE_ANNOUNCING,MDNS_STATE_COMPLETE };
struct mdns_host{int state,sent_num,rate_limit_activated;};
struct netif{int address;bool ipv6;struct mdns_host host;};
#define NETIF_TO_HOST(n) (&(n)->host)
#define netif_ip4_addr(n) (&(n)->address)
#define ip4_addr_isany_val(v) ((v)==0)
#define netif_ip6_addr_state(n,i) ((n)->ipv6)
#define ip6_addr_isvalid(v) (v)
static const int v4group=4,v6group=6;
static unsigned announcements,confirmed;
static void result(struct netif *n,int r,int s){(void)n;(void)s;assert(r==1);confirmed++;}
static void (*mdns_name_result_cb)(struct netif*,int,int)=result;
static int mdns_send_probe(struct netif *n,const int *group){return *group==4||n->ipv6?ERR_OK:-4;}
static void mdns_resp_announce(struct netif *n){(void)n;announcements++;}
static void sys_timeout(unsigned ms,void (*fn)(void*),void *arg){(void)ms;(void)fn;(void)arg;}
'''
checks=r'''
int main(void){
 struct netif n={.address=1};
 for(int i=0;i<5;i++)mdns_probe_and_announce(&n);
 assert(n.host.state==MDNS_STATE_COMPLETE&&announcements==2&&confirmed==1);
 n=(struct netif){.address=1,.ipv6=true};announcements=confirmed=0;
 for(int i=0;i<5;i++)mdns_probe_and_announce(&n);
 assert(n.host.state==MDNS_STATE_COMPLETE&&announcements==2&&confirmed==1);
 n=(struct netif){0};announcements=confirmed=0;
 for(int i=0;i<5;i++)mdns_probe_and_announce(&n);
 assert(!n.host.sent_num&&!announcements&&!confirmed);
 puts("Pinned mDNS probe: IPv4-only and dual-stack complete; absent IPv4 stays pending (simulated). ");
}
'''
with tempfile.TemporaryDirectory(prefix='stopwatch-mdns-') as d:
 c=pathlib.Path(d)/'probe.c';exe=pathlib.Path(d)/'probe'
 c.write_text(prelude+body+checks)
 subprocess.run([os.environ.get('CC','cc'),'-std=c11','-Wall','-Wextra','-Werror',str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
