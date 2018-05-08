
#include <ustack.h>
#include <ustack_eth.h>
#include <ustack_ipv4.h>
#include <ustack_icmp.h>
#include <ustack_udp.h>

// Debug
#include "serial.h"

void ustackIpv4ReplyHeader(ustack_iface_t *iface, uint8_t proto, uint8_t *dstMac, uint32_t dstIp)
{
  // Shortcut
  ipv4_t *ipv4 = (ipv4_t *)(iface->buffer + sizeof(eth_frame_t));
  
  ustackEthReplyHeader(iface, dstMac, USTACK_ETHER_IPV4);

  ipv4->version = USTACK_IPV4_VERSION;
  ipv4->ihl = 5;
  ipv4->dscp = 0;
  ipv4->ecn = 0;
  ipv4->identification = 0;
  ipv4->flags = USTACK_IPV4_FLAG_DONT_FRAGMENT;
  ipv4->fragOffsetL = 0;
  ipv4->fragOffsetH = 0;
  ipv4->proto = proto;
  ipv4->ttl = 64;
  ipv4->headerChksum = 0;

  if(dstIp != 0)
    ipv4->dstIp = htonl(dstIp);
  else
    ipv4->dstIp = ipv4->srcIp;

  ipv4->srcIp = htonl(iface->cfg->ip);
}

void ustackIpv4Finalize(ustack_iface_t *iface, uint16_t plen)
{
  //Shortcut
  ipv4_t *ipv4 = (ipv4_t *)(iface->buffer + sizeof(eth_frame_t));
  
  ipv4->totalLength = htons(sizeof(ipv4_t) + plen);
  ipv4->headerChksum = 0;
  ipv4->headerChksum = htons(ustackCalculateIpv4Checksum((uint8_t*)ipv4,sizeof(ipv4_t)));
}

void ustackIpv4Process(ustack_iface_t *iface, uint16_t len)
{
  //Shortcut
  ipv4_t *ipv4 = (ipv4_t*)(iface->buffer + sizeof(eth_frame_t));

  // Handle ICMP
  if(ipv4->proto == USTACK_PROTO_ICMP)
  {
    //dbg(":: USTACK :: Process ICMP");
    ustackIcmpProcess(iface, len);
  }
  else if(ipv4->proto == USTACK_PROTO_UDP)
  {
    //dbg(":: USTACK :: Process UDP");
    ustackUdpProcess(iface, len);
  }
  else if(ipv4->proto == USTACK_PROTO_TCP)
  {
    //dbg(":: USTACK :: Currently TCP Not supported");
    //ustackTcpProcess(iface);
  }
  /*
  else
  {
    dbgf(":: USTACK :: Unknown protocol %d\r\n",
         ipv4->proto);
  }
  */
}
