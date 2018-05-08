#include <ustack.h>
#include <ustack_ipv4.h>
#include <ustack_udp.h>

// Debug
#include "serial.h"

ustack_udp_listeners_t gUdpListeners[USTACK_UDP_MAX_LISTENERS] = {0};

void ustackUdpSend(ustack_iface_t *iface,
                   uint8_t *dstMac,
                   uint32_t dstIp,
                   uint16_t srcPort,
                   uint16_t dstPort,
                   uint16_t length)
{
  udp_t *udp = (udp_t*)(iface->buffer + sizeof(eth_frame_t) + sizeof(ipv4_t));
  
  ustackIpv4ReplyHeader(iface,
                        USTACK_PROTO_UDP,
                        dstMac,
                        dstIp);
  
  udp->srcPort = htons(srcPort);
  udp->dstPort = htons(dstPort);
  udp->length = htons(length + sizeof(udp_t));
  udp->chksum = 0;
  udp->chksum = htons(ustackCalculateUdpChecksum(iface, (uint8_t*)udp, length + sizeof(udp_t) ));

  ustackIpv4Finalize(iface, length + sizeof(udp_t));

  ustackSendPacket(length + sizeof(eth_frame_t) + sizeof(ipv4_t) + sizeof(udp_t));
}

bool ustackUdpAddListener(uint16_t port, ustackUdpCallback cb)
{
  if(port == 0) return false;
  uint8_t i;

  for(i = 0; i < USTACK_UDP_MAX_LISTENERS; i++)
  {
    if(gUdpListeners[i].port == port) // Update callback ?
    {
      gUdpListeners[i].cb = cb;
      return true;
    }

    if(gUdpListeners[i].port == 0)
    {
      gUdpListeners[i].port = port;
      gUdpListeners[i].cb = cb;
      return true;
    }
  }

  // Limit reached
  return false;
}

bool ustackUdpRemoveListener(uint16_t port)
{
  uint8_t i;

  for(i = 0; i < USTACK_UDP_MAX_LISTENERS; i++)
  {
    if(gUdpListeners[i].port == port)
    {
      gUdpListeners[i].port = 0;
      gUdpListeners[i].cb = NULL;
      return true;
    }
  }

  return false;
}

void ustackUdpProcess(ustack_iface_t *iface, uint16_t len)
{
  // We do not check for destination, if the applicatino reaches
  // this function, it's because it passed the initial filtering
  // mac address, and, thus, it's either for our mac or broadcast
  // so we pass it along and the callback needs to check if it's
  // intended for it or not
  udp_t *udp = (udp_t*)(iface->buffer + sizeof(eth_frame_t) + sizeof(ipv4_t));
  uint8_t i;

  for(i = 0; i < USTACK_UDP_MAX_LISTENERS; i++)
  {    
    if(gUdpListeners[i].port == htons(udp->dstPort))
    {
      if(gUdpListeners[i].cb != NULL)
        gUdpListeners[i].cb(iface, len);
    }
  }
}
