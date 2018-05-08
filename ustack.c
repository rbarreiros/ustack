
#include <ustack.h>
#include <ustack_ipv4.h>

// Debug
#include "debug.h"

/**
 *
 *
 * I realize we're developing for stm32, but, shouldn't we
 * check for endianess ? :/
 */
uint32_t ustackIpToA(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
  uint32_t r = 0;
  r = (a << 24) | (b << 16) | (c << 8) | d;
  return r;
}

void ustackAToIp(uint32_t ip, uint8_t *out)
{
  out[0] = (ip >> 24) & 0xff;
  out[1] = (ip >> 16) & 0xff;
  out[2] = (ip >> 8) & 0xff;
  out[3] = ip & 0xff;
}

uint16_t ustackCalculateChecksum(uint32_t sum, uint8_t *p, uint16_t len)
{
  //int32_t sum = 0;  // assume 32 bit long, 16 bit short 
  
  while (len > 1) {
    uint32_t p0 = *p;
    uint32_t p1 = *(p + 1);
    sum += (uint16_t) ((p0 << 8) | p1);
    len -= 2;
    p += 2;
  }
  
  if (len) {
    uint32_t p0 = *p;
    sum += p0 << 8;
  }
  
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }
  
  return (uint16_t)sum;
}

uint16_t ustackCalculateIpv4Checksum(uint8_t *p, uint16_t len)
{
  return ~(ustackCalculateChecksum(0, p, len));
}

uint16_t ustackCalculateUdpChecksum(ustack_iface_t *iface, uint8_t *p, uint16_t len)
{
  uint16_t sum = USTACK_PROTO_UDP + len;
  
  sum = ustackCalculateChecksum(sum, &iface->buffer[sizeof(eth_frame_t) + sizeof(ipv4_t) - 8], 8);
  sum = ustackCalculateChecksum(sum, p, len);

  return ~sum;
}

uint32_t ustackGetDirectedBroadcast(uint32_t ip, uint32_t netmask)
{
  return ((ip & netmask) | ~netmask);
}

bool ustackIsLinkUp(ustack_iface_t *iface)
{
  if(iface->link_cb != NULL)
    return iface->link_cb();

  return true;
}
