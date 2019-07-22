/*
 * Copyright (C) 2018 Silke Hofstra
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
 * @author      Silke Hofstra <silke@slxh.eu>
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
                uint8_t size_x, uint16_t size_y, il0373_entry_mode_t entry_mode)
{
    memcpy(&dev->params, params, sizeof(spi_epd_params_t));
    dev->size_x = size_x;
    dev->size_y = size_y;
    dev->entry_mode = entry_mode;
    dev->params.dummy = false;

    return spi_epd_init(&dev->params);
}

void il0373_display_init(il0373_t *dev)
{
    uint16_t y_data[2] = { 0 };

    y_data[0] = (dev->size_y - 1) & 0x1FF;
    spi_epd_write_cmd(&dev->params, IL0373_CMD_DRIVER_OUTPUT_CONTROL, (uint8_t *)y_data, 3);

    uint8_t data[3];
    data[0] = 0xD7; /* Phase 1: 30 ms phase, sel 3, 6.58 us off */
    data[1] = 0xD6; /* Phase 2: 30 ms phase, sel 3, 3.34 us off */
    data[2] = 0x9D; /* Phase 3: 10 ms phase, sel 4, 1.54 us off */
    spi_epd_write_cmd(&dev->params, IL0373_CMD_BOOSTER_SOFT_START_CONTROL, data, 3);

    data[0] = IL0373_VCOM;
    spi_epd_write_cmd(&dev->params, IL0373_CMD_WRITE_VCOM_REGISTER, data, 1);

    data[0] = 0x1A; /* 4 dummy line per gate */
    spi_epd_write_cmd(&dev->params, IL0373_CMD_SET_DUMMY_LINE_PERIOD, data, 1);

    data[0] = 0x08; /* 2 µs per line */
    spi_epd_write_cmd(&dev->params, IL0373_CMD_SET_GATE_LINE_WIDTH, data, 1);

    data[0] = (uint8_t)dev->entry_mode;
    spi_epd_write_cmd(&dev->params, IL0373_CMD_DATA_ENTRY_MODE_SETTING, data, 1);
}

void il0373_activate(il0373_t *dev)
{
    uint8_t data[] = { 0xC0 }; /* Enable Clock Signal then Enable CP */

    spi_epd_write_cmd(&dev->params, IL0373_CMD_DISPLAY_UPDATE_CONTROL_2, data, sizeof data);
    spi_epd_write_cmd(&dev->params, IL0373_CMD_MASTER_ACTIVATION, NULL, 0);
    spi_epd_wait(&dev->params, IL0373_WAIT_ACTIVATION);
}

void il0373_deactivate(il0373_t *dev)
{
    uint8_t data[] = { 0x03 }; /* Enable and disable (TODO: check C3 or 03) */

    spi_epd_write_cmd(&dev->params, IL0373_CMD_DISPLAY_UPDATE_CONTROL_2, data, sizeof data);
    spi_epd_write_cmd(&dev->params, IL0373_CMD_MASTER_ACTIVATION, NULL, 0);
    spi_epd_wait(&dev->params, IL0373_WAIT_ACTIVATION);
}

void il0373_init_full(il0373_t *dev)
{
    il0373_display_init(dev);
    il0373_set_area(dev, 0, 200, 0, 200);
    spi_epd_write_cmd(&dev->params, IL0373_CMD_WRITE_LUT_REGISTER,
                          il0373_lut_default_full, sizeof il0373_lut_default_full);
}

void il0373_update_full(il0373_t *dev)
{
    uint8_t data[] = { 0xC4 }; /* Enable Clock Signal then Enable CP | display pattern */

    spi_epd_write_cmd(&dev->params, IL0373_CMD_DISPLAY_UPDATE_CONTROL_2, data, sizeof data);
    spi_epd_write_cmd(&dev->params, IL0373_CMD_MASTER_ACTIVATION, NULL, 0);
    spi_epd_wait(&dev->params, IL0373_WAIT_UPDATE_FULL);
    spi_epd_write_cmd(&dev->params, IL0373_CMD_NOP, NULL, 0);
}

void il0373_init_part(il0373_t *dev)
{
    il0373_display_init(dev);
    il0373_set_area(dev, 0, 200, 0, 200);
    spi_epd_write_cmd(&dev->params, IL0373_CMD_WRITE_LUT_REGISTER,
                          il0373_lut_default_part, sizeof il0373_lut_default_part);
}

void il0373_update_part(il0373_t *dev)
{
    uint8_t data[] = { 0x04 }; /* To display pattern */

    spi_epd_write_cmd(&dev->params, IL0373_CMD_DISPLAY_UPDATE_CONTROL_2, data, sizeof data);
    spi_epd_write_cmd(&dev->params, IL0373_CMD_MASTER_ACTIVATION, NULL, 0);
    spi_epd_wait(&dev->params, IL0373_WAIT_UPDATE_PART);
    spi_epd_write_cmd(&dev->params, IL0373_CMD_NOP, NULL, 0);
}

void il0373_write_ram(il0373_t *dev)
{
    spi_epd_write_cmd(&dev->params, IL0373_CMD_WRITE_RAM, NULL, 0);
}

void il0373_clear(il0373_t *dev)
{
    il0373_fill(dev, 0, dev->size_x, 0, dev->size_y, IL0373_COLOR_WHITE);
}

void il0373_fill(il0373_t *dev, uint8_t x1, uint8_t x2, uint16_t y1, uint16_t y2, uint8_t color)
{
    il0373_set_area(dev, x1, x2, y1, y2);

    spi_acquire(dev->params.spi, dev->params.cs_pin, SPI_MODE_0, dev->params.spi_clk);
    spi_epd_cmd_start(&dev->params, IL0373_CMD_WRITE_RAM, true);

    uint16_t size = ((x2 - x1) >> 3) * (y2 - y1);
    for (uint16_t i = 0; i < size - 1; i++) {
        spi_transfer_byte(dev->params.spi, dev->params.cs_pin, true, color);
    }
    spi_transfer_byte(dev->params.spi, dev->params.cs_pin, false, color);

    spi_release(dev->params.spi);
}

void il0373_fill_pixels(il0373_t *dev, uint8_t x1, uint8_t x2, uint16_t y1, uint16_t y2,
                        uint8_t *px)
{
    il0373_set_area(dev, x1, x2, y1, y2);
    il0373_write_buffer(dev, px, (x2 - x1) * (y2 - y1));
}

void il0373_set_area(il0373_t *dev, uint8_t x1, uint8_t x2, uint16_t y1, uint16_t y2)
{
    /* Set X bounds */
    uint8_t x_data[] = {
        (x1 >> 3) & 0x1F,
        ((x2 - 1) >> 3) & 0x1F,
    };
    spi_epd_write_cmd(&dev->params, IL0373_CMD_SET_RAM_X, x_data, sizeof x_data);

    /* Set Y bounds */
    le_uint16_t y_data[] = {
        byteorder_btols(byteorder_htons(y1 & 0x01FF)),
        byteorder_btols(byteorder_htons((y2 - 1) & 0x01FF)),
    };
    spi_epd_write_cmd(&dev->params, IL0373_CMD_SET_RAM_Y, (uint8_t *)y_data, sizeof y_data);

    /* Set counters to start positions */
    spi_epd_write_cmd(&dev->params, IL0373_CMD_SET_RAM_X_ADDR_COUNTER, x_data, 1);
    spi_epd_write_cmd(&dev->params, IL0373_CMD_SET_RAM_Y_ADDR_COUNTER, (uint8_t *)y_data, 2);
}

void il0373_write_buffer(il0373_t *dev, const uint8_t *buf, size_t len)
{
    spi_epd_write_cmd(&dev->params, IL0373_CMD_WRITE_RAM, buf, len);
}

void il0373_sleep(il0373_t *dev)
{
    uint8_t data = 0x01;

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

    /* Turn off sleep mode */
    uint8_t data = 0x00;
    spi_epd_write_cmd(&dev->params, IL0373_CMD_DEEP_SLEEP_MODE, &data, 1);
}

void il0373_swreset(il0373_t *dev)
{
    spi_epd_write_cmd(&dev->params, IL0373_CMD_SWRESET, NULL, 0);
}
