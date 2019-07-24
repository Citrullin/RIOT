/*
 * Copyright (C) 2019 Philipp-Alexander Blum
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_il0373  IL0373 E-ink/E-paper controller
 * @ingroup     drivers_actuators
 * @brief       Device driver for the il0373 E-ink/E-paper display controller.
 *
 * The IL0373 is a display controller for black, white and colored E-ink/E-paper displays.
 * It is commonly found in 2.13 and 2.9 inch displays.
 *
 * More information:
 * - [IL0373 datasheet (Good Display)](http://www.e-paper-display.com/download_detail/downloadsId=535.html)
 *
 * ## Module usage
 *
 * Add the following to your makefile.
 * Note that not all display boards have the *reset* and *busy* pins exposed.
 *
 * ```make
 * USEMODULE += il0373
 *
 * # Set the following to the correct pins
 * CFLAGS += -DIL0373_PARAM_CS=GPIO_PIN\(PORT_A,1\)
 * CFLAGS += -DIL0373_PARAM_DC=GPIO_PIN\(PORT_A,2\)
 * CFLAGS += -DIL0373_PARAM_RST=GPIO_PIN\(PORT_A,3\) # if RST is exposed
 * CFLAGS += -DIL0373_PARAM_BUSY=GPIO_PIN\(PORT_A,4\) # if BUSY is exposed
 * ```
 *
 * This allows initialisation using `il0373_params[0]` or `IL0373_PARAMS`.
 *
 * The driver includes the RIOT logo in various sizes in `il0373_pictures.h`.
 *
 * ## Driver usage
 *
 * A write cycle to the display looks like the following.
 * Substitute `full` by `part` for a partial refresh.
 *
 * ```c
 * il0373_init_full(dev);
 * il0373_activate(dev);
 * il0373_write_buffer(dev, riot_icon_200, sizeof riot_icon_200);
 * il0373_update_full(dev);
 * il0373_deactivate(dev);
 * ```
 *
 * The IL0373 has two memory buffers for storing the images.
 * After an update (either partial or full), the controller switches to the
 * other buffer.
 *
 * @{
 * @file
 * @brief       Device driver for the il0373 display controller
 *
 * @author      Silke Hofstra <silke@slxh.eu>
 */

#ifndef IL0373_H
#define IL0373_H

#include "periph/spi.h"
#include "periph/gpio.h"
#include "spi_epd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IL0373_COLOR_WHITE_VALUE (0xFF) /**< White (8x1 pixels) */
#define IL0373_COLOR_BLACK_VALUE (0x00) /**< Black (8x1 pixels) */
#define IL0373_COLOR_COLORED_VALUE (0x00) /**< Red or Yellow (8x1 pixels) */
#define IL0373_COLOR_NON_COLORED_VALUE (0xFF) /**< Non Red or Yellow (8x1 pixels) */

/**
 * @brief Color
 */
typedef enum {
    IL0373_COLOR_WHITE,
    IL0373_COLOR_BLACK,
    IL0373_COLOR_COLORED /** RED or YELLOW. Depends on the used panel.*/
} il0373_color_t;

/**
 * @brief   Device initialisation parameters.
 */
typedef struct {
    spi_epd_params_t params;        /**< SPI display parameters */
    uint8_t size_x;                 /**< number of horizontal pixels in the display */
    uint16_t size_y;                /**< number of vertical pixels in the display */
} il0373_t;

/**
 * @brief   Initialise the display.
 *
 * @param[out] dev          Display to initialise.
 * @param[in]  params       SPI Display parameters to use for initialisation.
 * @param[in]  size_x       Number of horizontal pixels in the display.
 * @param[in]  size_y       Number of vertical pixels in the display.
 */
int il0373_init(il0373_t *dev, const spi_epd_params_t *params,
                uint8_t size_x, uint16_t size_y);

/**
 * @brief   Initialise the display for a full refresh.
 *
 * @param[in] dev   Device descriptor.
 */
void il0373_init_full(il0373_t *dev);

/**
 * @brief   Initialise the display for a partial refresh.
 *
 * @param[in] dev   Device descriptor.
 */
void il0373_init_part(il0373_t *dev);

/**
 * @brief   Clear the entire display.
 *
 * @param[in] dev   Device descriptor.
 */
void il0373_clear(il0373_t *dev);

/**
 * @brief   Fill an area with a single color.
 *
 * @param[in] dev   Device descriptor
 * @param[in] color Color to use (`IL0373_COLOR_BLACK` or `IL0373_COLOR_WHITE`)
 */
void il0373_fill(il0373_t *dev, uint8_t color);

/**
 * @brief   Fill an area with an array of pixels.
 *
 * Note that the length of the array should be the same as the number of pixels
 * in the given area.
 *
 * @param[in] dev           Device descriptor.
 * @param[in] px_black      Array of black/white pixels to use.
 * @param[in] px_colored    Array of colored (yellow or red)/none pixels to use.
 */
void il0373_fill_pixels(il0373_t *dev, uint8_t *px_black, uint8_t *px_colored, uint16_t length);

/**
 * @brief   Set the area in which can be drawn.
 *
 * @param[in] dev   Device descriptor.
 * @param[in] x1    X coordinate of the first corner (multiple of 8).
 * @param[in] x2    X coordinate of the opposite corner (multiple of 8).
 * @param[in] y1    Y coordinate of the first corner.
 * @param[in] y2    Y coordinate of the opposite corner.
 */
void il0373_set_area(il0373_t *dev, uint8_t x1, uint8_t x2, uint16_t y1, uint16_t y2);

/**
 * @brief   Set the display to deep sleep mode.
 *
 * After the display has gone to sleep, a wake can be triggered with the reset pin.
 *
 * @param[in] dev   Device descriptor.
 */
void il0373_sleep(il0373_t *dev);

/**
 * @brief   Wake the device.
 *
 * This doesn't do anything without using the reset pin.
 *
 * @param[in] dev   Device descriptor.
 */
void il0373_wake(il0373_t *dev);

#ifdef __cplusplus
}
#endif
#endif /* IL0373_H */
/** @} */
