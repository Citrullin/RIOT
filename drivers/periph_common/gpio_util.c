/*
<<<<<<< HEAD
 * Copyright (C) 2017 Philipp-Alexander Blum <philipp-blum@jakiku.de>
=======
 * Copyright (C) 2019 Philipp-Alexander Blum <philipp-blum@jakiku.de>
>>>>>>> master
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "periph/gpio.h"
<<<<<<< HEAD

#ifdef MODULE_PERIPH_GPIO

uint8_t gpio_util_shiftin(gpio_t data_pin, gpio_t clock_pin){
    uint8_t byte = 0x00;

    for(unsigned int i = 8; i > 0; i--) {
        gpio_set(clock_pin);

        if(gpio_read(data_pin) > 0) {
            byte |= (1 << (i - 1));
        }

=======
#include <stdint.h>

#ifdef MODULE_PERIPH_GPIO

uint8_t gpio_util_shiftin(gpio_t data_pin, gpio_t clock_pin)
{
    uint8_t byte = 0x00;

    for (unsigned int i = 8; i > 0; i--) {
        gpio_set(clock_pin);
        if (gpio_read(data_pin) > 0) {
            byte |= (1 << (i - 1));
        }
>>>>>>> master
        gpio_clear(clock_pin);
    }

    return byte;
}
<<<<<<< HEAD
#endif
/*MODULE_PERIPH_GPIO*/
=======

#endif /* MODULE_PERIPH_GPIO */
>>>>>>> master
