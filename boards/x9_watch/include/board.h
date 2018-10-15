/*
 * Copyright (C) 2016-2017 Feie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_nrf52dk
 * @{
 *
 * @file
 * @brief       Board specific configuration for the nRF52 DK
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Sebastian Meiling <s@mlng.net>
 */

#ifndef BOARD_H
#define BOARD_H

#include "board_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    LED pin configuration
 * @{
 */
#define LED0_PIN            GPIO_PIN(0, 4)


/** @} */

/**
 * @name    Button pin configuration
 * @{
 */

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
