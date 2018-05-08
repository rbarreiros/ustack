#include <ustack.h>
#include <ustack_eth.h>
#include <ustack_ipv4.h>
#include <ustack_arp.h>
#include <ustack_icmp.h>
#include <ustack_udp.h>
#include <ustack_thread.h>

#include <ch.h>
#include <hal.h>
#include <evtimer.h>

// Debug
#include "debug.h"

#define EVID_NET_INT      EVENT_MASK(1) // Interrupt event
#define EVID_NET_PERIODIC EVENT_MASK(2) // Periodic network event
#define EVID_NET_ARP      EVENT_MASK(3) // ARP event
#define EVID_NET_SEND     EVENT_MASK(4) // Sporadic packet send

static ustack_iface_t gIface = {0};
static EVENTSOURCE_DECL(gIntEvtSrc);
static EVENTSOURCE_DECL(gPktSendEvtSrc);
static ustackQueuePacketCallback_t gPacketcb = NULL;

static void recvInterrupt(void *arg)
{
  (void)arg;
  chSysLockFromISR();
  chEvtBroadcastI(&gIntEvtSrc);
  chSysUnlockFromISR();
}

bool ustackQueueSendPacket(ustackQueuePacketCallback_t cb)
{
  if(gPacketcb == NULL)
  {
    gPacketcb = cb;

    //chSysLock();
    chEvtBroadcast(&gPktSendEvtSrc);
    //chSysUnlock();

    return true;
  }

  return false;
}

void ustackSendPacket(uint16_t len)
{
  if(gIface.send_cb != NULL)
    gIface.send_cb(gIface.buffer, len);
}

static THD_WORKING_AREA(waUSTACK, USTACK_THREAD_SIZE);
static __attribute__((noreturn)) THD_FUNCTION(ustackThread, arg)
{
  (void)arg;

  chRegSetThreadName("UStack Thread");
  
  event_timer_t per_evt, arp_evt;
  event_listener_t int_el, per_el, arp_el, pkt_el;
  eventmask_t evt_mask;
  uint16_t recv_len = 0;

  // Configure external interrupt
  // This now is enc28j60 specific, need to abstrack this
  // to receive events from the enc driver instead of
  // generating them here
  palSetPadMode(GPIO_ENC_PORT, GPIO_ENC_INT, PAL_MODE_INPUT);
  // Disable this channel events to make sure there's nothing lingering
  palDisablePadEvent(GPIO_ENC_PORT, GPIO_ENC_INT);
  // Enable interrupt
  palEnablePadEvent(GPIO_ENC_PORT, GPIO_ENC_INT, PAL_EVENT_MODE_FALLING_EDGE);
  palSetPadCallback(GPIO_ENC_PORT, GPIO_ENC_INT, recvInterrupt, NULL);

  evtObjectInit(&per_evt, TIME_MS2I(500));
  evtObjectInit(&arp_evt, TIME_S2I(10));

  evtStart(&per_evt);
  evtStart(&arp_evt);

  // Register Events
  chEvtRegisterMask(&gIntEvtSrc, &int_el, EVID_NET_INT);
  chEvtRegisterMask(&gPktSendEvtSrc, &pkt_el, EVID_NET_SEND);
  chEvtRegisterMask(&per_evt.et_es, &per_el, EVID_NET_PERIODIC);
  chEvtRegisterMask(&arp_evt.et_es, &arp_el, EVID_NET_ARP);

  chEvtAddEvents(EVID_NET_INT | EVID_NET_PERIODIC | EVID_NET_ARP | EVID_NET_SEND);

  uint8_t out[4];
  dbg("Thread uStack started with config:");
  dbgf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
       gIface.cfg->mac[0], gIface.cfg->mac[1], gIface.cfg->mac[2], gIface.cfg->mac[3], gIface.cfg->mac[4], gIface.cfg->mac[5]);

  ustackAToIp(gIface.cfg->ip, out);
  dbgf("IP: %d.%d.%d.%d\r\n", out[0], out[1], out[2], out[3]);
  ustackAToIp(gIface.cfg->netmask, out);
  dbgf("Netmask: %d.%d.%d.%d\r\n", out[0], out[1], out[2], out[3]);
  ustackAToIp(gIface.cfg->gateway, out);
  dbgf("Gateway: %d.%d.%d.%d\r\n\r\n", out[0], out[1], out[2], out[3]);
  
  while(true)
  {
    // If int is still active, keep flagging for int flag
    if(!palReadPad(GPIO_ENC_PORT, GPIO_ENC_INT))
      evt_mask = EVID_NET_INT;
    else
      evt_mask = chEvtWaitAny(ALL_EVENTS);
  
    if(evt_mask == EVID_NET_INT)
    {
      if(gIface.recv_cb != NULL)
      {
        recv_len = gIface.recv_cb(gIface.buffer, USTACK_BUFFER_SIZE);
        if(recv_len > 0)
        {         
          eth_frame_t *eth = (eth_frame_t*)gIface.buffer;
  
          if(ntohs(eth->etherType) == USTACK_ETHER_ARP)
            ustackArpProcess(&gIface, recv_len);
          else if(ntohs(eth->etherType) == USTACK_ETHER_IPV4)
            ustackIpv4Process(&gIface, recv_len);
        }
      }
    }
    else if(evt_mask == EVID_NET_SEND)
    {
      if(gPacketcb != NULL)
      {
        gPacketcb(&gIface);
        gPacketcb = NULL;
      }
    }
    else if(evt_mask == EVID_NET_PERIODIC)
    {
    }
    else if(evt_mask == EVID_NET_ARP)
    {
    }
  }
}

ustack_iface_t *ustackInit(USTACKConfig *cfg,
                           ustackSendCallback_t send_cb,
                           ustackRecvCallback_t recv_cb,
                           ustackLinkUpCallback_t link_cb)
{
  gIface.cfg = cfg;
  gIface.send_cb = send_cb;
  gIface.recv_cb = recv_cb;
  gIface.link_cb = link_cb;
  
  chThdCreateStatic(waUSTACK, sizeof(waUSTACK), NORMALPRIO + 1, ustackThread, NULL);
  
  return &gIface;
}
