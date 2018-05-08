#include <ustack.h>
#include <ustack_eth.h>
#include <ustack_arp.h>

#include <string.h>
#include <ch.h>

// Debug
#include "debug.h"

static ustack_arp_table_t gArpTable[USTACK_ARP_TABLE_SIZE] = {0};

static void ustackArpHandleReply(ustack_iface_t *iface)
{
  arp_t *arp = (arp_t*)(iface->buffer + sizeof(eth_frame_t));
  
  // Not for us
  if(memcmp(iface->cfg->mac, arp->dstMac, 6) != 0)
    return;

  uint8_t i;

  for(i = 0; i < USTACK_ARP_TABLE_SIZE; i++)
  {
    if(arp->srcIp != gArpTable[i].ip)
      continue;
    else
    {
      if(memcmp(arp->srcMac, gArpTable[i].mac, 6) == 0)
      {
        gArpTable[i].time = chVTGetSystemTimeX();
        return;
      }
    }
  }

  for(i = 0; i < USTACK_ARP_TABLE_SIZE; i++)
  {
    if(gArpTable[i].ip == 0 && gArpTable[i].time == 0)
    {
      gArpTable[i].ip = arp->srcIp;
      memcpy(gArpTable[i].mac, arp->srcMac, 6);
      gArpTable[i].time = chVTGetSystemTimeX();
      return;
    }
  }
}

static void ustackArpHandleRequest(ustack_iface_t *iface)
{
  // Shortcut
  arp_t *arp = (arp_t*)(iface->buffer + sizeof(eth_frame_t));

  // Is it for us ?
  if(iface->cfg->ip != ntohl(arp->dstIp))
    return;
  
  // Send reply
  ustackEthReplyHeader(iface, arp->srcMac, USTACK_ETHER_ARP);
  memcpy(arp->dstMac, arp->srcMac, 6);
  memcpy(arp->srcMac, iface->cfg->mac, 6);

  arp->dstIp = arp->srcIp;
  arp->srcIp = htonl(iface->cfg->ip);
  arp->opcode = htons(USTACK_ARP_OP_REPLY);

  /*
  uint8_t ip[4];
  ustackAToIp(htonl(arp->dstIp), ip);
  dbgf(":: ARP :: Sending ARP reply to : %02x:%02x:%02x:%02x:%02x:%02x - %d.%d.%d.%d\r\n",
       arp->dstMac[0],
       arp->dstMac[1],
       arp->dstMac[2],
       arp->dstMac[3],
       arp->dstMac[4],
       arp->dstMac[5],
       ip[0],
       ip[1],
       ip[2],
       ip[3]
       );
  */
  ustackSendPacket(sizeof(eth_frame_t) + sizeof(arp_t));
}

void ustackArpSendRequest(ustack_iface_t *iface, uint32_t requestedIpAddress)
{
  // IMPORTANT !!! WE SHOULD NOT PROCESS ANYTHING
  // DURING THIS FUNCTION CALL
  
  // Build an ARP request packet, clean the
  // packet space used by the new packet

  // Shortcut
  arp_t *arp = (arp_t*)(iface->buffer + sizeof(eth_frame_t));
  eth_frame_t *mac = (eth_frame_t*)iface->buffer;
  uint8_t i;
  
  memset(iface->buffer, 0, sizeof(arp_t) + sizeof(eth_frame_t));

  for(i = 0; i < 6; i++)
  {
    mac->dstMac[i] = 0xff; // Broadcast
    mac->srcMac[i] = iface->cfg->mac[i];
    arp->dstMac[i] = 0x00;
    arp->srcMac[i] = iface->cfg->mac[i];
  }

  mac->etherType = htons(USTACK_ETHER_ARP);
  arp->hwType = htons(USTACK_ARP_HW_TYPE_ETHERNET);
  arp->protoType = htons(USTACK_ETHER_IPV4);
  arp->hwAddressLength = 6;
  arp->protoAddressLength = 4;
  arp->opcode = USTACK_ARP_OP_REQUEST;
  arp->srcIp = iface->cfg->ip;
  arp->dstIp = requestedIpAddress;

  ustackSendPacket(sizeof(struct _arp));
}

void ustackArpProcess(ustack_iface_t *iface, uint16_t len)
{
  // Shortcut
  arp_t *arp = (arp_t*)(iface->buffer + sizeof(eth_frame_t));
  (void)len;
  
  // Ethernet only
  if(ntohs(arp->hwType) != USTACK_ARP_HW_TYPE_ETHERNET)
    return;

  // IPv4 only
  if(ntohs(arp->protoType) != USTACK_ETHER_IPV4)
    return;
  
  // Mac address bigger than 6 ? nop!
  if(arp->hwAddressLength > 6)
    return;

  // IP bigger than 4 ? nop!
  if(arp->protoAddressLength > 4)
    return;

  uint16_t op = ntohs(arp->opcode);
  if(op == USTACK_ARP_OP_REQUEST)
  {
    //dbg(":: ARP :: Received ARP Request");
    ustackArpHandleRequest(iface);
  }
  else if(op == USTACK_ARP_OP_REPLY)
  {
    dbg(":: ARP :: Received ARP Reply");
    ustackArpHandleReply(iface);
  }
  else
  {
    dbgf("!! ARP :: Invalid ARP opcode 0x%04x", op);
  }
}
