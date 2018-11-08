/*
 * Copyright (C) 2018 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup   drivers_cc110x
 * @{
 *
 * @file
 * @brief     Functions to communicate with the CC1100/CC1101 transceiver
 *
 * @author    Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 * @}
 */

#ifndef CC110X_INTERNAL_H
#define CC110X_INTERNAL_H

#include "cc110x_constants.h"
#include "cc110x_settings.h"
#include "cc110x_communication.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RIOT's API to interact with the CC1100/CC1101 driver
 */
extern const netdev_driver_t cc110x_driver;

/**
 * @brief ISR to be called via @ref netdev_driver_t::isr
 */
void cc110x_isr(netdev_t *dev);

/**
 * @brief Interrupt handler to call on both edges of the GDO0 and GDO2 pins
 *
 * @param dev     The device descriptor of the transceiver
 *
 * This interrupt handler requests that the cc110x ISR is called in thread
 * context
 */
void cc110x_on_gdo(void *dev);

/**
 * @brief Perform a recalibration of the transceiver
 *
 * @param dev     The device descriptor of the transceiver
 *
 * @retval 0      Success
 * @retval -EIO   Failed
 *
 * @pre           @p dev has been acquired using @ref cc110x_acquire
 * @pre           Transceiver is in IDLE state
 * @post          @p dev is still acquired, the caller has to release it
 * @post          Transceiver is again in IDLE state, calibration has been
 *                performed and calibration data has been backed up on MCU.
 */
int cc110x_recalibrate(cc110x_t *dev);

/**
 * @brief Bring transceiver into RX mode
 *
 * @param dev     The device descriptor of the transceiver
 *
 * @pre           @p dev has been acquired using @ref cc110x_acquire
 * @post          @p dev is still acquired, the caller has to release it
 */
void cc110x_rx(cc110x_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* CC110X_INTERNAL_H */
/** @} */
