#ifndef __USTACK_ICMP_H__
#define __USTACK_ICMP_H__

#include <ustack_thread.h>

#define USTACK_ICMP_TYPE_REPLY 0
#define USTACK_ICMP_TYPE_REQUEST 8

void ustackIcmpProcess(ustack_iface_t *iface, uint16_t len);

#endif
