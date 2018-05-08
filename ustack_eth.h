#ifndef __USTACK_ETH_H__
#define __USTACK_ETH_H__

#include <ustack_thread.h>

#define USTACK_ETHER_IPV4 0x0800
#define USTACK_ETHER_ARP  0x0806

void ustackEthReplyHeader(ustack_iface_t *iface, uint8_t *dstMac, uint16_t type);

#endif
