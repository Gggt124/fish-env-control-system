#include "netif_helper.h"
#include "esp_log.h"
#include "esp_netif_net_stack.h"
#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/ip4_addr.h"
#include "lwip/etharp.h"
#include "lwip/prot/etharp.h"
#include "lwip/prot/ieee.h"
#include "netif/ethernet.h"
#include <string.h>

static const char *TAG = "netif_helper";

static struct netif s_alias_netif;
static ip4_addr_t s_alias_ip;
static netif_input_fn s_orig_sta_input = NULL;

static err_t alias_netif_init(struct netif *netif)
{
    netif->name[0] = 'e';
    netif->name[1] = 'a';
    netif->output = etharp_output;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_LINK_UP;
    return ERR_OK;
}

static err_t sta_input_hook(struct pbuf *p, struct netif *inp)
{
    struct eth_hdr *eth = (struct eth_hdr *)p->payload;

    if (eth->type == PP_HTONS(ETHTYPE_IP)) {
        struct ip_hdr *iphdr = (struct ip_hdr *)((u8_t *)p->payload + SIZEOF_ETH_HDR);
        if (IPH_V(iphdr) == 4) {
            ip4_addr_t dest;
            ip4_addr_copy(dest, iphdr->dest);
            if (ip4_addr_cmp(&dest, &s_alias_ip)) {
                return ethernet_input(p, &s_alias_netif);
            }
        }
        return s_orig_sta_input(p, inp);
    }

    if (eth->type == PP_HTONS(ETHTYPE_ARP)) {
        struct etharp_hdr *arp = (struct etharp_hdr *)((u8_t *)p->payload + SIZEOF_ETH_HDR);
        if (arp->opcode == PP_HTONS(ARP_REQUEST)) {
            ip4_addr_t arp_target;
            IPADDR_WORDALIGNED_COPY_TO_IP4_ADDR_T(&arp_target, &arp->dipaddr);
            if (ip4_addr_cmp(&arp_target, &s_alias_ip)) {
                return ethernet_input(p, &s_alias_netif);
            }
        }
        return s_orig_sta_input(p, inp);
    }

    return s_orig_sta_input(p, inp);
}

esp_err_t netif_add_alias_ip(esp_netif_t *sta_netif, const char *ip_str)
{
    struct netif *lwip_sta = esp_netif_get_netif_impl(sta_netif);
    if (!lwip_sta) {
        ESP_LOGE(TAG, "STA lwIP netif not found");
        return ESP_FAIL;
    }

    ip4_addr_t ip, netmask, gw;
    ip4addr_aton(ip_str, &ip);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    ip4_addr_copy(s_alias_ip, ip);

    memset(&s_alias_netif, 0, sizeof(s_alias_netif));

    s_orig_sta_input = lwip_sta->input;

    struct netif *ret = netif_add(&s_alias_netif, &ip, &netmask, &gw, NULL,
                                  alias_netif_init, ethernet_input);
    if (!ret) {
        ESP_LOGE(TAG, "Failed to add alias netif");
        s_orig_sta_input = NULL;
        return ESP_FAIL;
    }

    memcpy(s_alias_netif.hwaddr, lwip_sta->hwaddr, ETH_HWADDR_LEN);
    s_alias_netif.hwaddr_len = ETH_HWADDR_LEN;
    s_alias_netif.linkoutput = lwip_sta->linkoutput;

    netif_set_up(&s_alias_netif);

    lwip_sta->input = sta_input_hook;

    ESP_LOGI(TAG, "Added alias IP %s on STA", ip_str);
    return ESP_OK;
}
