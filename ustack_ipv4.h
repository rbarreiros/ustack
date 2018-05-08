#ifndef __USTACK_IPV4_H__
#define __USTACK_IPV4_H__

#include <ustack_thread.h>

#define USTACK_IPV4_VERSION 4
#define USTACK_IPV4_FLAG_DONT_FRAGMENT  0x02
#define USTACK_IPV4_FLAG_MORE_FRAGMENTS 0x01

#define USTACK_PROTO_ICMP 0x01
#define USTACK_PROTO_TCP  0x06
#define USTACK_PROTO_UDP  0x11

void ustackIpv4Process(ustack_iface_t *iface, uint16_t len);
void ustackIpv4ReplyHeader(ustack_iface_t *iface, uint8_t proto, uint8_t *dstMac, uint32_t dstIp);
void ustackIpv4Finalize(ustack_iface_t *iface, uint16_t plen);

#endif
