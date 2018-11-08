/*
 * Copyright (C) 2016-17 Freie Universität Berlin
 *               2018 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gnrc_netif
 * @{
 *
 * @file
 *
 * @author      Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 *
 * Adapted from @ref gnrc_cc110x.c with only minor modifications. So all credit
 * goes to Martine Lenders <m.lenders@fu-berlin.de> and to
 * Hauke Petersen <hauke.petersen@fu-berlin.de>.
 *
 * @}
 */

#include "assert.h"
#include "cc110x.h"
#include "net/gnrc.h"
#include "gnrc_netif_cc110x.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#define BCAST  (GNRC_NETIF_HDR_FLAGS_BROADCAST | GNRC_NETIF_HDR_FLAGS_MULTICAST)

static gnrc_pktsnip_t *cc110x_adpt_recv(gnrc_netif_t *netif)
{
    netdev_t *dev = netif->dev;
    cc110x_rx_info_t rx_info;
    int pktlen;
    gnrc_pktsnip_t *payload;
    gnrc_pktsnip_t *netif_snip;
    gnrc_pktsnip_t *xhdr_snip;
    gnrc_netif_hdr_t *netif_hdr;
    cc110x_l2hdr_t l2hdr;

    assert(dev);

    /* see how much data there is to process */
    pktlen = dev->driver->recv(dev, NULL, 0, &rx_info);
    if (pktlen <= 0) {
        DEBUG("[cc110x-gnrc] recv: no data available to process\n");
        return NULL;
    }

    /* allocate space for the packet in the pktbuf */
    payload = gnrc_pktbuf_add(NULL, NULL, pktlen, CC110X_DEFAULT_PROTOCOL);
    if (payload == NULL) {
        DEBUG("[cc110x-gnrc] recv: unable to allocate space in the pktbuf\n");
        /* tell the driver to drop the packet */
        dev->driver->recv(dev, NULL, 1, NULL);
        return NULL;
    }

    /* copy the complete including the CC110X header into the packet buffer */
    dev->driver->recv(dev, payload->data, pktlen, NULL);

    /* The first two bytes are the layer 2 header */
    l2hdr.dest_addr = ((uint8_t *)payload->data)[0];
    l2hdr.src_addr = ((uint8_t *)payload->data)[1];

    /* crop the layer 2 header from the payload */
    xhdr_snip = gnrc_pktbuf_mark(payload, CC110X_HEADER_SIZE, GNRC_NETTYPE_UNDEF);
    if (xhdr_snip == NULL) {
        DEBUG("[cc110x-gnrc] recv: unable to mark CC110x header snip\n");
        gnrc_pktbuf_release(payload);
        return NULL;
    }
    gnrc_pktbuf_remove_snip(payload, xhdr_snip);

    /* create a netif hdr from the obtained data */
    netif_snip = gnrc_netif_hdr_build(&l2hdr.src_addr, CC110X_ADDR_SIZE,
                                      &l2hdr.dest_addr, CC110X_ADDR_SIZE);
    if (netif_snip == NULL) {
        DEBUG("[cc110x-gnrc] recv: unable to allocate netif header\n");
        gnrc_pktbuf_release(payload);
        return NULL;
    }
    netif_hdr = (gnrc_netif_hdr_t *)netif_snip->data;
    netif_hdr->if_pid = netif->pid;
    netif_hdr->rssi = rx_info.rssi;
    netif_hdr->lqi = rx_info.lqi;
    if (l2hdr.dest_addr == 0x00) {
        netif_hdr->flags = GNRC_NETIF_HDR_FLAGS_BROADCAST;
    }

    DEBUG("[cc110x-gnrc] recv: successfully parsed packet\n");

    /* and append the netif header */
    LL_APPEND(payload, netif_snip);

    return payload;
}

static int cc110x_adpt_send(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt)
{
    int res;
    size_t size;
    cc110x_t *cc110x_dev = (cc110x_t *)netif->dev;
    gnrc_netif_hdr_t *netif_hdr;
    cc110x_l2hdr_t l2hdr;

    /* check device descriptor and packet */
    assert(netif && pkt);

    /* get the payload size and the dst address details */
    size = gnrc_pkt_len(pkt->next);
    DEBUG("[cc110x-gnrc] send: payload of packet is %i\n", (int)size);
    netif_hdr = (gnrc_netif_hdr_t *)pkt->data;


    l2hdr.src_addr = cc110x_dev->addr;
    if (netif_hdr->flags & BCAST) {
        l2hdr.dest_addr = 0x00;
        DEBUG("[cc110x-gnrc] send: preparing to send broadcast\n");
    }
    else {
        /* check that destination address is valid */
        assert(netif_hdr->dst_l2addr_len > 0);
        uint8_t *addr = gnrc_netif_hdr_get_dst_addr(netif_hdr);
        l2hdr.dest_addr = addr[netif_hdr->dst_l2addr_len - 1];
        DEBUG("[cc110x-gnrc] send: preparing to send unicast %02x --> %02x\n",
              (int)l2hdr.src_addr, (int)l2hdr.dest_addr);
    }

    /* now let's send out the stuff */
    iolist_t iolist = {
        .iol_next = (iolist_t *)pkt->next,
        .iol_base = &l2hdr,
        .iol_len = sizeof(l2hdr),
    };

#ifdef MODULE_NETSTATS_L2
    if (netif_hdr->flags & BCAST) {
        netif->dev->stats.tx_mcast_count++;
    }
    else {
        netif->dev->stats.tx_unicast_count++;
    }
#endif
    DEBUG("[cc110x-gnrc] send: triggering the drivers send function\n");
    res = netif->dev->driver->send(netif->dev, &iolist);

    gnrc_pktbuf_release(pkt);

    return res;
}

static const gnrc_netif_ops_t cc110x_netif_ops = {
    .send = cc110x_adpt_send,
    .recv = cc110x_adpt_recv,
    .get = gnrc_netif_get_from_netdev,
    .set = gnrc_netif_set_from_netdev,
};

gnrc_netif_t *gnrc_netif_cc110x_create(char *stack, int stacksize,
                                       char priority, char *name,
                                       netdev_t *dev)
{
    return gnrc_netif_create(stack, stacksize, priority, name,
                             dev, &cc110x_netif_ops);
}
