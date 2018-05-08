#ifndef __USTACK_THREAD_H__
#define __USTACK_THREAD_H__

#include <ch.h>

#define USTACK_THREAD_SIZE 1024

typedef void (*ustackSendCallback_t)(uint8_t *buffer, uint16_t len);
typedef uint16_t (*ustackRecvCallback_t)(uint8_t *buffer, uint16_t maxsize);
typedef bool (*ustackLinkUpCallback_t)(void);

typedef struct USTACKConfig
{
  uint8_t mac[6];
  uint32_t ip;
  uint32_t netmask;
  uint32_t gateway;
} USTACKConfig;

typedef struct ustack_iface_t
{
  USTACKConfig *cfg;
  ustackSendCallback_t send_cb;
  ustackRecvCallback_t recv_cb;
  ustackLinkUpCallback_t link_cb;
  uint8_t buffer[USTACK_BUFFER_SIZE];
} ustack_iface_t;

typedef void (*ustackQueuePacketCallback_t)(ustack_iface_t *iface);

ustack_iface_t *ustackInit(USTACKConfig *cfg,
                           ustackSendCallback_t send_cb,
                           ustackRecvCallback_t recv_cb,
                           ustackLinkUpCallback_t link_cb);
void ustackSendPacket(uint16_t len);
bool ustackQueueSendPacket(ustackQueuePacketCallback_t cb);

#endif
