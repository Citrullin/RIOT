/*
 * Copyright (C) 2018 Silke Hofstra, Philipp-Alexander Blum
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_spi_epd
 *
 * @{
 * @file
 * @brief       Common device driver elements for SPI driven epaper displays (epd)
 *
 * @author      Silke Hofstra <silke@slxh.eu>, Philipp-Alexander Blum <philipp-blum@jakiku.de>
 */
#include "periph/spi.h"
#include "xtimer.h"
#include "spi_epd.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

void spi_epd_cmd_start(spi_epd_params_t *p, uint8_t cmd, bool cont)
{
    if (p->busy_pin != GPIO_UNDEF) {
        DEBUG("[spi_epd] device is busy\n");
        while (gpio_read(p->busy_pin) == p->busy_pin_active) {}
    }
    gpio_clear(p->dc_pin);
    DEBUG("[spi_epd] device not busy. Transfer command byte [0x%x].\n", cmd & 0xff);
    spi_transfer_byte(p->spi, p->cs_pin, cont, (uint8_t)cmd);
    gpio_set(p->dc_pin);
}

void spi_epd_write_cmd(spi_epd_params_t *p, uint8_t cmd, const uint8_t *params, size_t plen)
{
    DEBUG("[spi_epd] write cmd\n");
    spi_acquire(p->spi, p->cs_pin, SPI_MODE_3, p->spi_clk);
    spi_epd_cmd_start(p, cmd, plen ? true : false);
    if (plen) {
        spi_transfer_bytes(p->spi, p->cs_pin, false, params, NULL, plen);
    }
    spi_release(p->spi);
}

void spi_epd_read_cmd(spi_epd_params_t *p, uint8_t cmd, uint8_t *params, size_t plen)
{
    spi_acquire(p->spi, p->cs_pin, SPI_MODE_0, p->spi_clk);
    spi_epd_cmd_start(p, cmd, true);
    if (p->dummy) {
        spi_transfer_byte(p->spi, p->cs_pin, true, 0x00);
    }
    spi_transfer_bytes(p->spi, p->cs_pin, false, NULL, params, plen);
    spi_release(p->spi);
}

int spi_epd_init(spi_epd_params_t *p)
{
    if (p->rst_pin != GPIO_UNDEF) {
        if (gpio_init(p->rst_pin, GPIO_OUT) != 0) {
            DEBUG("[spi_epd] init: error initializing the RST pin\n");
            return SPI_DISPLAY_RST_FAIL;
        }
        gpio_set(p->rst_pin);
    }

    if (p->busy_pin != GPIO_UNDEF) {
        if (gpio_init(p->busy_pin, GPIO_IN) != 0) {
            DEBUG("[spi_epd] init: error initializing the BUSY pin\n");
            return SPI_DISPLAY_BUSY_FAIL;
        }
    }

    if (gpio_init(p->dc_pin, GPIO_OUT) != 0) {
        DEBUG("[spi_epd] init: error initializing the DC pin\n");
        return SPI_DISPLAY_DC_FAIL;
    }
    gpio_set(p->dc_pin);

    int res = spi_init_cs(p->spi, p->cs_pin);
    if (res != SPI_OK) {
        DEBUG("[spi_epd] init: error initializing the CS pin [%i]\n", res);
        return res;
    }

    return 0;
}

void spi_epd_wait(spi_epd_params_t *p, uint32_t usec)
{
    if (p->busy_pin != GPIO_UNDEF) {
        DEBUG("[spi_epd] wait for busy == %i\n", p->busy_pin_active);
        while (gpio_read(p->busy_pin) == p->busy_pin_active) {}
        DEBUG("[spi_epd] controller is ready\n");
    }
    else {
        xtimer_usleep(usec);
    }
}
