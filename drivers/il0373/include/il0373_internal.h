/*
 * Copyright (C) 2019 Philipp-Alexander Blum
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_il0373
 * @{
 *
 * @file
 * @brief       Device driver implementation for the il0373 display controller
 *
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * @}
 */

#ifndef IL0373_INTERNAL_H
#define IL0373_INTERNAL_H

#include "xtimer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    IL0373 SPI commands
 * @{
 */
#define IL0373_CMD_PANEL_SETTING (0x00)
#define IL0373_CMD_POWER_SETTING (0x01)
#define IL0373_CMD_POWER_OFF (0x02)
#define IL0373_CMD_POWER_OFF_SEQUENCE (0x03)
#define IL0373_CMD_POWER_ON (0x04)
#define IL0373_CMD_POWER_ON_MEASURE (0x05)
#define IL0373_CMD_BOOSTER_SOFT_START (0x06)
#define IL0373_CMD_DEEP_SLEEP (0x07)
#define IL0373_CMD_DTM1 (0x10)
#define IL0373_CMD_DATA_STOP (0x11)
#define IL0373_CMD_DISPLAY_REFRESH (0x12)
#define IL0373_CMD_PDTM1 (0x14)
#define IL0373_CMD_PDTM2 (0x15)
#define IL0373_CMD_PDRF (0x16)
#define IL0373_CMD_LUT1 (0x20)
#define IL0373_CMD_LUTWW (0x21)
#define IL0373_CMD_LUTBW (0x22)
#define IL0373_CMD_LUTWB (0x23)
#define IL0373_CMD_LUTBB (0x24)
#define IL0373_CMD_PLL (0x30)
#define IL0373_CMD_CDI (0x50)
#define IL0373_CMD_RESOLUTION (0x61)
#define IL0373_CMD_VCM_DC_SETTING (0x82)
/** @} */

/**
 * @name    IL0373 Waiting estimates in microseconds
 * @{
 */
#define IL0373_WAIT_UPDATE_FULL (1200 * US_PER_MS)
#define IL0373_WAIT_UPDATE_PART (300 * US_PER_MS)
#define IL0373_WAIT_ACTIVATION  (80 * US_PER_MS)
#define IL0373_WAIT_RESET       (1 * US_PER_MS)
/** @} */

/**
 * @name    IL0373 lookup table for a full display refresh
 */
static const uint8_t il0373_lut_default_full[] = {
        0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22, 0x66, 0x69,
        0x69, 0x59, 0x58, 0x99, 0x99, 0x88, 0x00, 0x00, 0x00, 0x00,
        0xF8, 0xB4, 0x13, 0x51, 0x35, 0x51, 0x51, 0x19, 0x01, 0x00};

/**
 * @name    IL0373 lookup table for a partial display refresh
 */
static const uint8_t il0373_lut_default_part[] = {
        0x10, 0x18 ,0x18 ,0x08, 0x18, 0x18, 0x08, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x13, 0x14, 0x44, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/**
 * @name    Common display initialisation code
 */
static void il0373_display_init(il0373_t *dev);

#ifdef __cplusplus
}
#endif
#endif /* IL0373_INTERNAL_H */
