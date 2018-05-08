#include <ustack.h>
#include <ustack_eth.h>
#include <ustack_thread.h>
#include <string.h>

void ustackEthReplyHeader(ustack_iface_t *iface, uint8_t *dstMac, uint16_t type)
{
  eth_frame_t *mac = (eth_frame_t*)iface->buffer;

  if(dstMac == NULL)
    memcpy(mac->dstMac, mac->srcMac, 6);
  else
    memcpy(mac->dstMac, dstMac, 6);
  
  memcpy(mac->srcMac, iface->cfg->mac, 6);
  mac->etherType = htons(type);
}
