#ifndef __USTACK_H__
#define __USTACK_H__

#include <stdint.h>
#include <stdbool.h>

// Configuration

#ifndef USTACK_BUFFER_SIZE
#define USTACK_BUFFER_SIZE 1520
#endif

#ifndef USTACK_ARP_TABLE_SIZE
#define USTACK_ARP_TABLE_SIZE 5
#endif

#ifndef USTACK_MAX_TCP_LISTENERS
#define USTACK_MAX_TCP_LISTENERS 5
#endif

#ifndef USTACK_MAX_UDP_LISTENERS
#define USTACK_MAX_UDP_LISTENERS 5
#endif

#include <ustack_thread.h>

// Defines

#ifndef htons
#define htons(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#endif

#ifndef ntohs
#define ntohs(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#endif

#ifndef htonl
#define htonl(n) (((((unsigned long)(n) & 0xFF)) << 24) |               \
                  ((((unsigned long)(n) & 0xFF00)) << 8) |              \
                  ((((unsigned long)(n) & 0xFF0000)) >> 8) |            \
                  ((((unsigned long)(n) & 0xFF000000)) >> 24))
#endif

#ifndef ntohl
#define ntohl(n) (((((unsigned long)(n) & 0xFF)) << 24) |               \
                  ((((unsigned long)(n) & 0xFF00)) << 8) |              \
                  ((((unsigned long)(n) & 0xFF0000)) >> 8) |            \
                  ((((unsigned long)(n) & 0xFF000000)) >> 24))
#endif


typedef struct _eth_frame
{
  uint8_t dstMac[6];
  uint8_t srcMac[6];
  uint16_t etherType;
} __attribute__((packed)) eth_frame_t;

typedef struct _icmp
{
  uint8_t icmpType;
  uint8_t code;
  uint16_t chksum;
  uint16_t identifier;
  uint16_t seq;
} __attribute__((packed)) icmp_t;

typedef struct _arp
{
  uint16_t hwType;            // Hardware type
  uint16_t protoType;         // Protocol type
  uint8_t hwAddressLength;
  uint8_t protoAddressLength;
  uint16_t opcode;
  uint8_t srcMac[6];
  uint32_t srcIp;
  uint8_t dstMac[6];
  uint32_t dstIp;
} __attribute__((packed)) arp_t;

typedef struct _ipv4
{
  uint8_t ihl: 4;
  uint8_t version: 4;
  uint8_t dscp: 6;
  uint8_t ecn: 2;
  uint16_t totalLength;
  uint16_t identification;
  uint8_t fragOffsetH: 5;
  uint8_t flags: 3;
  uint8_t fragOffsetL;
  uint8_t ttl;
  uint8_t proto;
  uint16_t headerChksum;
  uint32_t srcIp;
  uint32_t dstIp;
} __attribute__((packed)) ipv4_t;

typedef struct _udp
{
  uint16_t srcPort;
  uint16_t dstPort;
  uint16_t length;
  uint16_t chksum;
} __attribute__((packed)) udp_t;

typedef union ustack_network_packet_u
{
  eth_frame_t eth_frame;
  
  struct
  {
    eth_frame_t mac;
    arp_t arp;
  } __attribute__((packed)) arp;

  struct
  {
    eth_frame_t mac;
    ipv4_t ipv4;
  } __attribute__((packed)) ipv4;
  
  struct
  {
    eth_frame_t mac;
    ipv4_t ipv4;
    icmp_t icmp;
  } __attribute__((packed)) icmp;

  struct
  {
    eth_frame_t mac;
    ipv4_t ipv4;
    udp_t udp;
  }__attribute__((packed)) udp;

  uint8_t buffer[USTACK_BUFFER_SIZE];
} ustack_network_packet_u;


uint32_t ustackIpToA(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void ustackAToIp(uint32_t ip, uint8_t *out);
uint16_t ustackCalculateChecksum(uint32_t sum, uint8_t *data, uint16_t length);
uint16_t ustackCalculateIpv4Checksum(uint8_t *p, uint16_t len);
uint16_t ustackCalculateUdpChecksum(ustack_iface_t *iface, uint8_t *data, uint16_t length);
uint32_t ustackGetDirectedBroadcast(uint32_t ip, uint32_t netmask);
bool ustackIsLinkUp(ustack_iface_t *iface);
#endif
