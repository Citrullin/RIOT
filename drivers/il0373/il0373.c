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
 * @brief       Device driver implementation for the il0373 display controller
 *
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 */
#include <string.h>
#include "byteorder.h"
#include "spi_epd.h"

#include "il0373.h"
#include "il0373_params.h"
#include "il0373_internal.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

int il0373_init(il0373_t *dev, const spi_epd_params_t *params,
                uint8_t size_x, uint16_t size_y)
{
    memcpy(&dev->params, params, sizeof(spi_epd_params_t));
    dev->size_x = size_x;
    dev->size_y = size_y;
    dev->params.dummy = false;

    return spi_epd_init(&dev->params);
}

void il0373_display_init(il0373_t *dev)
{
    il0373_wake(dev);

    uint8_t data[3];
    data[0] = 0x17;
    data[1] = 0x17;
    data[2] = 0x17;
    spi_epd_write_cmd(&dev->params, IL0373_CMD_BOOSTER_SOFT_START_CONTROL, data, 3);

    spi_epd_cmd_start(&dev->params, IL0373_CMD_POWER_ON, false);

    //Todo: Make configurable
    data[0] = 0x0f;
    spi_epd_write_cmd(&dev->params, IL0373_CMD_PANEL_SETTING, data, 1);

    uint32_t res_data[1] = { 0 };

    res_data[0] = ((dev->size_x << 19) ^ dev->size_y) & 0xFE01FF;
    spi_epd_write_cmd(&dev->params, IL0373_CMD_RESOLUTION, (uint8_t *)res_data, 3);

    data[0] = 0x77;
    spi_epd_write_cmd(&dev->params, IL0373_CMD_CDI, data, 1);
}

void il0373_init_full(il0373_t *dev)
{
    il0373_display_init(dev);
    spi_epd_write_cmd(&dev->params, IL0373_CMD_LEAVE_PARTIAL_MODE, NULL, 0);
}

void il0373_init_part(il0373_t *dev)
{
    il0373_display_init(dev);
    spi_epd_write_cmd(&dev->params, IL0373_CMD_ENTER_PARTIAL_MODE, NULL, 0);

    il0373_set_area(dev, 0, 13, 0, 200);
}

void il0373_clear(il0373_t *dev)
{
    il0373_fill(dev, IL0373_COLOR_WHITE);
}

void il0373_fill(il0373_t *dev, il0373_color_t color)
{
    uint8_t black_white_fill_data = IL0373_COLOR_WHITE_VALUE;
    uint8_t colored_fill_data = IL0373_COLOR_COLORED_VALUE;

    if(color == IL0373_COLOR_COLORED){
        black_white_fill_data = IL0373_COLOR_WHITE_VALUE;
        colored_fill_data = IL0373_COLOR_COLORED_VALUE;
    }else if(color == IL0373_COLOR_BLACK){
        black_white_fill_data = IL0373_COLOR_BLACK_VALUE;
        colored_fill_data = IL0373_COLOR_COLORED_VALUE;
    }

    spi_acquire(dev->params.spi, dev->params.cs_pin, SPI_MODE_0, dev->params.spi_clk);

    spi_epd_cmd_start(&dev->params, IL0373_CMD_DTM1, true);
    uint16_t size = (DISPLAY_X >> 3) * DISPLAY_Y;
    for (uint16_t i = 0; i < size; i++) {
        spi_transfer_byte(dev->params.spi, dev->params.cs_pin, true, black_white_fill_data);
    }

    spi_epd_cmd_start(&dev->params, IL0373_CMD_DTM2, true);
    for (uint16_t i = 0; i < size; i++) {
        spi_transfer_byte(dev->params.spi, dev->params.cs_pin, true, colored_fill_data);
    }

    spi_epd_cmd_start(&dev->params, IL0373_CMD_DATA_STOP, false);

    spi_release(dev->params.spi);
}

void il0373_fill_pixels(il0373_t *dev, uint8_t *px_black, uint8_t *px_colored, uint16_t length)
{
    spi_epd_write_cmd(&dev->params, IL0373_CMD_DTM1, px_black, length);
    spi_epd_write_cmd(&dev->params, IL0373_CMD_DTM2, px_colored, length);

    spi_epd_cmd_start(&dev->params, IL0373_CMD_DATA_STOP, false);
}

void il0373_set_area(il0373_t *dev, uint8_t x1, uint8_t x2, uint16_t y1, uint16_t y2)
{
    uint8_t data[7];

    data[0] = x1 << 3;
    data[1] = x2 << 3;

    data[2] = (uint8_t) (y1 >> 7) & 0x01;
    data[3] = (uint8_t) y1;

    data[4] = (uint8_t) (y2 >> 7) & 0x01;
    data[5] = (uint8_t) y2;

    data[6] = 0x01;

    spi_epd_write_cmd(&dev->params, IL0373_CMD_PARTIAL_WINDOW, data, sizeof(data));
}

void il0373_sleep(il0373_t *dev)
{
    uint8_t data = 0xA5;
    spi_epd_write_cmd(&dev->params, IL0373_CMD_DEEP_SLEEP_MODE, &data, 1);
}

/* TODO: test this */
void il0373_wake(il0373_t *dev)
{
    /* Give a low pulse on the reset pin */
    if (dev->params.rst_pin != GPIO_UNDEF) {
        gpio_clear(dev->params.rst_pin);
        spi_epd_wait(&dev->params, IL0373_WAIT_RESET);
        gpio_set(dev->params.rst_pin);
    }
}

