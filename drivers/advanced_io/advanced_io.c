/*
 * Copyright (C) 2017 Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "periph/gpio.h"

uint8_t advanced_io_shift_in(gpio_t data_pin, gpio_t clock_pin){
    uint8_t byte = 0x00;

    for(unsigned int i = 0; i <= 8; i++){
        gpio_set(clock_pin);
        if(gpio_read(data_pin) > 0){
            byte = byte | 0x01 << (8 - i);
        }
        gpio_clear(clock_pin);
    }

    return byte;
}
