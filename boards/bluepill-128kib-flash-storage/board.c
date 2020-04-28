/*
 * Copyright (C) 2015 TriaGnoSys GmbH
 *               2017 Alexander Kurth, Sören Tempel, Tristan Bruns
 *               2020 Philipp-Alexander Blum
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_bluepill
 * @{
 *
 * @file
 * @brief       Board specific implementations for the bluepill board with spi flash storage
 *
 * @author      Víctor Ariño <victor.arino@triagnosys.com>
 * @author      Sören Tempel <tempel@uni-bremen.de>
 * @author      Tristan Bruns <tbruns@uni-bremen.de>
 * @author      Alexander Kurth <kurth1@uni-bremen.de>
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * @}
 */

#include <stddef.h> /* for NULL */
#include <stdio.h>
#include "board.h"
#include "periph/gpio.h"
#include "vfs.h"
#include "fs/devfs.h"
#include "mtd_spi_nor.h"
#include "periph/spi.h"

static mtd_spi_nor_t bluepill_nor_dev = {
        .base = {
                .driver = &mtd_spi_nor_driver,
                .page_size = 256,
                .pages_per_sector = 256,
                .sector_count = 128,
        },
        .opcode = &mtd_spi_nor_opcode_default,
        .spi = BLUEPILL_NOR_SPI_DEV,
        .cs = BLUEPILL_NOR_SPI_CS,
        .addr_width = 3,
        .mode = SPI_MODE_3,
        .clk = SPI_CLK_10MHZ,

};

mtd_dev_t *mtd0 = (mtd_dev_t *)&bluepill_nor_dev;

static devfs_t bluepill_nor_devfs = {
        .path = "/mtd0",
        .f_op = &mtd_vfs_ops,
        .private_data = &bluepill_nor_dev,
};

int bluepill_nor_init(void);

void board_init(void)
{
    cpu_init();
    gpio_init(LED0_PIN, GPIO_OUT);

    /* Initialize NOR flash */
    bluepill_nor_init();
}

int bluepill_nor_init(void)
{
    int res = mtd_init(mtd0);

    if (res >= 0) {
        /* Register DevFS node */
        devfs_register(&bluepill_nor_devfs);
    }

    return res;
}
