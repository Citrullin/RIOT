/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *               2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     sys_auto_init_gnrc_netif
 * @{
 *
 * @file
 * @brief       Auto initialization for cc110x network interfaces
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifdef MODULE_CC110X

#include "log.h"
#include "cc110x.h"
#include "gnrc_netif_cc110x.h"
#include "cc110x_params.h"
#define ENABLE_DEBUG    (0)
#include "debug.h"

/**
 * @brief   Calculate the number of configured CC1100/CC1101 transceivers
 */
#define CC110X_NUM        (sizeof(cc110x_params) / sizeof(cc110x_params_t))

/**
 * @brief   Define stack parameters for the MAC layer thread
 */
#define CC110X_MAC_STACKSIZE           (THREAD_STACKSIZE_DEFAULT + DEBUG_EXTRA_STACKSIZE)
#ifndef CC110X_MAC_PRIO
#define CC110X_MAC_PRIO                (GNRC_NETIF_PRIO)
#endif

/**
 * @brief   Allocate memory for device descriptors, stacks, and GNRC adaption
 */
cc110x_t _cc110x_devs[CC110X_NUM];
static char stacks[CC110X_NUM][CC110X_MAC_STACKSIZE];

void auto_init_cc110x(void)
{
    for (unsigned i = 0; i < CC110X_NUM; i++) {
        LOG_DEBUG("[auto_init_netif] initializing cc110x #%u\n", i);

        cc110x_setup(&_cc110x_devs[i], &cc110x_params[i]);
        gnrc_netif_cc110x_create(stacks[i], CC110X_MAC_STACKSIZE, CC110X_MAC_PRIO,
                                 "cc110x", (netdev_t *)&_cc110x_devs[i]);
    }
}

#else
typedef int dont_be_pedantic;
#endif /* MODULE_CC110X */
/** @} */
