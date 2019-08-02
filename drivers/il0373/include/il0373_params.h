/*
 * Copyright (C) 2019 Philipp-Alexander Blum
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_il0373
 *
 * @{
 * @file
 * @brief       Default configuration for IL0373
 *
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 */

#ifndef IL0373_PARAMS_H
#define IL0373_PARAMS_H

#include "board.h"
#include "il0373.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Default parameters for IL0373 display */
/**
 * @name    Set default configuration parameters for the IL0373
 * @{
 */
#ifndef IL0373_PARAM_SPI
#define IL0373_PARAM_SPI          (SPI_DEV(1))
#endif
#ifndef IL0373_PARAM_SPI_CLK
#define IL0373_PARAM_SPI_CLK      (SPI_CLK_5MHZ)
#endif
#ifndef IL0373_PARAM_CS
#define IL0373_PARAM_CS           (GPIO_UNDEF)
#endif
#ifndef IL0373_PARAM_DC
#define IL0373_PARAM_DC           (GPIO_UNDEF)
#endif
#ifndef IL0373_PARAM_RST
#define IL0373_PARAM_RST          (GPIO_UNDEF)
#endif
#ifndef IL0373_PARAM_BUSY
#define IL0373_PARAM_BUSY         (GPIO_UNDEF)
#endif

#ifndef IL0373_PARAMS
#define IL0373_PARAMS              { .spi = IL0373_PARAM_SPI, \
                                     .spi_clk = IL0373_PARAM_SPI_CLK, \
                                     .cs_pin = IL0373_PARAM_CS, \
                                     .dc_pin = IL0373_PARAM_DC, \
                                     .rst_pin = IL0373_PARAM_RST, \
                                     .busy_pin = IL0373_PARAM_BUSY, \
                                     .busy_pin_active = SPI_EPD_ACTIVE_LOW, \
                                   }
#endif
/**@}*/

/**
 * @brief   Configure IL0373
 */
static const spi_epd_params_t il0373_params[] =
{
    IL0373_PARAMS,
};

#ifdef __cplusplus
}
#endif
#endif /* IL0373_PARAMS_H */
