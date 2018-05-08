
#include <ustack.h>
#include <ustack_ipv4.h>
#include <ustack_icmp.h>
#include <ustack_thread.h>

// Debug
#include "serial.h"

static void ustackIcmpHandleRequest(ustack_iface_t *iface, uint16_t len)
{
  // Shortcuts
  icmp_t *icmp = (icmp_t*)(iface->buffer + sizeof(eth_frame_t) + sizeof(ipv4_t));
  ipv4_t *ipv4 = (ipv4_t*)(iface->buffer + sizeof(eth_frame_t));
  eth_frame_t *mac = (eth_frame_t*)iface->buffer;

  // Packet len (icmp size + extra data)
  // Len has the whole packet size
  uint16_t plen = len - sizeof(eth_frame_t) - sizeof(ipv4_t);
  
  ustackIpv4ReplyHeader(iface,
                        ipv4->proto,
                        mac->srcMac,
                        ntohl(ipv4->srcIp));

  icmp->icmpType = USTACK_ICMP_TYPE_REPLY;
  icmp->code = 0;
  icmp->chksum = 0;
  icmp->chksum = htons(ustackCalculateChecksum(0, (uint8_t*)icmp, plen));

  ustackIpv4Finalize(iface, plen);
  ustackSendPacket(len);

  /*
  uint8_t ip[4];
  ustackAToIp(ntohl(ipv4->dstIp), ip);
  dbgf(":: ICMP :: Sending reply to %d.%d.%d.%d\r\n",
       ip[0], ip[1], ip[2], ip[3]);
  */
}

void ustackIcmpProcess(ustack_iface_t *iface, uint16_t len)
{
  icmp_t *icmp = (icmp_t*)(iface->buffer + sizeof(eth_frame_t) + sizeof(ipv4_t));
  
  if(icmp->icmpType == USTACK_ICMP_TYPE_REQUEST)
    ustackIcmpHandleRequest(iface, len);
  else
    dbgf(":: ICMP :: Unhandled ICMP type 0x%02x\r\n", icmp->icmpType);
}
