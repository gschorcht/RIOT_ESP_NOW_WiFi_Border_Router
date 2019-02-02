/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example application for demonstrating the RIOT network stack
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include <net/gnrc/ipv6/nib.h>
#include <net/gnrc/ipv6.h>
#include <net/gnrc/netapi.h>
#include <net/gnrc/netif.h>
#ifdef MODULE_GNRC_RPL
#include <net/gnrc/rpl.h>
#endif
#include <net/ethernet.h>
#include <net/ipv6/addr.h>
#include <net/netdev.h>
#include <net/netopt.h>
#include <xtimer.h>
#include <shell.h>
#include <msg.h>

static msg_t main_msg_queue[16];

static gnrc_netif_t *outer_interface = NULL;
static gnrc_netif_t *inner_interface = NULL;

static int find_interfaces(void)
{
    uint16_t mtu;
    gnrc_netif_t *netif = NULL;

    outer_interface = inner_interface = NULL;

    while ((netif = gnrc_netif_iter(netif))) {
        gnrc_netapi_get(netif->pid, NETOPT_MAX_PACKET_SIZE, 0, &mtu, sizeof(mtu));

        if (!outer_interface && (mtu == ETHERNET_DATA_LEN)) {
            outer_interface = netif;
        } else if (!inner_interface && (mtu != ETHERNET_DATA_LEN)) {
            inner_interface = netif;
        }

        if (outer_interface && inner_interface)
            break;
    }

    if (!outer_interface || !inner_interface) {
        printf("Unable to find interfaces.\n");
        return -1;
    }

    return 0;
}

static int set_ips(void)
{
#if defined(BR_IPV6_ADDR) && defined(BR_IPV6_ADDR_LEN)
    /* Add configured outer address */
    ipv6_addr_t addr;
    ipv6_addr_from_str(&addr, BR_IPV6_ADDR);
    if (gnrc_netif_ipv6_addr_add(outer_interface, &addr, BR_IPV6_ADDR_LEN, 0) < 0) {
        printf("Failed setting outer address.\n");
        return -1;
    }
#endif

#ifdef BR_IPV6_DEF_RT
    /* Add default route */
    ipv6_addr_t defroute = IPV6_ADDR_UNSPECIFIED;
    ipv6_addr_from_str(&addr, BR_IPV6_DEF_RT);
    if (gnrc_ipv6_nib_ft_add(&defroute, 0, &addr, outer_interface->pid, 0) < 0) {
        printf("Failed setting default route.\n");
        return -1;
    }
#endif

    /* Turn off router advert on outer interface, it's really not our job. */
    gnrc_ipv6_nib_change_rtr_adv_iface(outer_interface, false);

    /* Add inner address based on prefix and interface iid */
    eui64_t iid;
    ipv6_addr_t prefix;
    ipv6_addr_from_str(&prefix, BR_IPV6_PREFIX);
    if (gnrc_netapi_get(inner_interface->pid, NETOPT_IPV6_IID, 0, &iid, sizeof(iid)) < 0) {
        printf("Failed getting wireless interface iid.\n");
        return -1;
    }
    ipv6_addr_set_aiid(&prefix, iid.uint8);
    if (gnrc_netif_ipv6_addr_add(inner_interface, &prefix, 64, 0) < 0) {
        printf("Failed setting outer address.\n");
        return -1;
    }

#if GNRC_IPV6_NIB_CONF_MULTIHOP_P6C
    /* Add as authoritative border router */
    if (gnrc_ipv6_nib_abr_add(&prefix) < 0) {
        printf("Failed adding prefix as authoritative border router.\n");
        return -1;
    }
#endif

#ifdef MODULE_GNRC_RPL
    /* Configure rpl */
    if (gnrc_rpl_init(inner_interface->pid) < 0) {
        printf("Failed initializing RPL on inner wireless interface.\n");
        return -1;
    }
    gnrc_rpl_instance_t *inst = gnrc_rpl_instance_get(GNRC_RPL_DEFAULT_INSTANCE);
    if (inst) {
        gnrc_rpl_instance_remove(inst);
    }
    if (!gnrc_rpl_root_init(GNRC_RPL_DEFAULT_INSTANCE, &prefix, false, false)) {
        printf("Failed initializing RPL root.\n");
        return -1;
    }
#endif

    return 0;
}

int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(main_msg_queue, sizeof(main_msg_queue) / sizeof(main_msg_queue[0]));
    puts("RIOT border router example application");

    if (find_interfaces() >= 0)
    {
        set_ips();
    }

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
