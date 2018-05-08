#ifndef __USTACK_ARP_H__
#define __USTACK_ARP_H__

#include <ustack.h>
#include <ustack_thread.h>


#define USTACK_ARP_HW_TYPE_ETHERNET  0x0001
#define USTACK_ARP_OP_REQUEST 0x1
#define USTACK_ARP_OP_REPLY   0x2

typedef struct ustack_arp_table_t
{
  uint8_t mac[6];
  uint32_t ip;
  systime_t time;
} ustack_arp_table_t;

void ustackArpProcess(ustack_iface_t *iface, uint16_t len);

#endif
