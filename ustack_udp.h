#ifndef __USTACK_UDP__
#define __USTACK_UDP__

#include <ustack_thread.h>

#define USTACK_UDP_MAX_LISTENERS 5

typedef void (*ustackUdpCallback)(ustack_iface_t *iface, uint16_t len);

typedef struct _ustack_udp_listeners_t
{
  uint16_t port;
  ustackUdpCallback cb;
} ustack_udp_listeners_t;

void ustackUdpProcess(ustack_iface_t *iface, uint16_t len);
bool ustackUdpAddListener(uint16_t port, ustackUdpCallback cb);
bool ustackUdpRemoveListener(uint16_t port);
void ustackUdpSend(ustack_iface_t *iface, uint8_t *dstMac, uint32_t dstIp,
                   uint16_t srcPort, uint16_t dstPort, uint16_t length);

#endif
