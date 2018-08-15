<<<<<<< HEAD
/*
 * Copyright (C) 2017 Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "periph/gpio.h"

typedef enum {
    MSBFIRST,
    LSBFIRST
} advanced_io_bit_order_t;

uint8_t advanced_io_shift_in(gpio_t data_pin, gpio_t clock_pin, advanced_io_bit_order_t bit_order){
    uint8_t byte = 0x00;

    for(int i = 0; i <= 8; i++){
        gpio_set(clock_pin);
        if(gpio_read(data_pin) > 0){
            if(bit_order == MSBFIRST){
                byte = byte | 0x01 << (8 - i);
            }
            else if(bit_order == LSBFIRST){
                byte = byte | 0x01 << i;
            } else {
                return '\0';
            }
        }

        gpio_clear(clock_pin);
    }

    return byte;
}
