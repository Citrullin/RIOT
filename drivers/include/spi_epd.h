/*
 * Copyright (C) 2018 Silke Hofstra
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_spi_epd Driver elements for SPI driven epaper displays (epd)
 * @ingroup     drivers_actuators
 * @brief       Common device driver elements for SPI driven epaper displays (epd)
 *
 * @{
 * @file
 * @brief       Common device driver elements for SPI driven epaper displays (epd)
 *
 * @author      Silke Hofstra <silke@slxh.eu>
 */
#ifndef SPI_EPD_H
#define SPI_EPD_H

#include "periph/spi.h"
#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   SPI epaper display device initialisation parameters.
 */
typedef struct {
    spi_t spi;          /**< SPI device that the display is connected to */
    spi_clk_t spi_clk;  /**< SPI clock speed to use */
    gpio_t cs_pin;      /**< pin connected to the CHIP SELECT line */
    gpio_t dc_pin;      /**< pin connected to the DC line */
    gpio_t rst_pin;     /**< pin connected to the reset line */
    gpio_t busy_pin;    /**< pin connected to the busy line */
    bool dummy;         /**< if device requires a dummy cycle before read */
} spi_epd_params_t;

/**
 * @brief   Additional status codes for SPI displays.
 */
enum {
    SPI_DISPLAY_DC_FAIL = -5,
    SPI_DISPLAY_RST_FAIL = -6,
    SPI_DISPLAY_BUSY_FAIL = -7,
};

/**
 * @brief   Initialise a SPI epaper display from parameters.
 *
 * @param[in] p         SPI epaper display parameters.
 */
int spi_epd_init(spi_epd_params_t *p);

/**
 * @brief   Send a SPI epaper display command with data.
 *
 * This function requires that `spi_acquire()` has been called.
 *
 * @param[in] p         SPI display parameters.
 * @param[in] cmd       Command to execute.
 * @param[in] cont      Continue with data after this command.
 */
void spi_epd_cmd_start(spi_epd_params_t *p, uint8_t cmd, bool cont);

/**
 * @brief   Send a SPI epaper display command with data.
 *
 * @param[in] p         SPI epd parameters.
 * @param[in] cmd       Command to execute.
 * @param[in] params    Command parameters to send.
 * @param[in] plen      Size of the command parameters.
 */
void spi_epd_write_cmd(spi_epd_params_t *p, uint8_t cmd, const uint8_t* params, size_t plen);

/**
 * @brief   Send a SPI epaper display command and store the returned data.
 *
 * @param[in] p         SPI epd parameters.
 * @param[in] cmd       Command to execute.
 * @param[in] params    Command response to read.
 * @param[in] plen      Size of the command response.
 */
void spi_epd_read_cmd(spi_epd_params_t *p, uint8_t cmd, uint8_t* params, size_t plen);

/**
 * @brief   Wait for the busy pin or a predetermined amount of time.
 *
 * @param[in] p         SPI epd parameters.
 * @param[in] usec      Number of µs to wait if the busy pin is undefined.
 */
void spi_epd_wait(spi_epd_params_t *p, uint32_t usec);

#ifdef __cplusplus
}
#endif
#endif /* SPI_EPD_H */
/** @} */
