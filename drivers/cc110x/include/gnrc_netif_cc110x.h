/*
 * Copyright (C) 2017 Freie Universität Berlin
 *               2018 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup net_gnrc_netif
 * @{
 *
 * @file
 * @brief   CC110x adaption for @ref net_gnrc_netif
 *
 * @author  Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 *
 * Adapted from @ref gnrc_netif_xbee.h with only minor modifications. So all
 * credit goes to Martine Lenders <m.lenders@fu-berlin.de>.
 */
#ifndef GNRC_NETIF_CC110X_H
#define GNRC_NETIF_CC110X_H

#include "net/gnrc/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

gnrc_netif_t *gnrc_netif_cc110x_create(char *stack, int stacksize,
                                       char priority, char *name,
                                       netdev_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* GNRC_NETIF_CC110X_H */
/** @} */
