<<<<<<< HEAD
/*
 * Copyright (C) 2017 Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup   advanced_io Advanced I/O Utils
 * @ingroup    utils
 * @brief      Advanced I/O Utils
 *
 *             The implementation is Arduino compatible with a different API.
 * @{
 *
 * @file
 * @brief      Advanced I/O Utils
 *
 * @author     Philipp-Alexander Blum <philipp-blum@jakiku.de>
 */

#ifdef __cplusplus
extern "C" {
#endif

=======
>>>>>>> hx711_implementation
typedef enum {
    MSBFIRST,
    LSBFIRST
} advanced_io_bit_order_t;

<<<<<<< HEAD
uint8_t advanced_io_shift_in(gpio_t dataPin, gpio_t clockPin, advanced_io_bit_order_t bitOrder);

#ifdef __cplusplus
}
#endif
=======
uint8_t advanced_io_shift_in(gpio_t dataPin, gpio_t clockPin, advanced_io_bit_order_t bitOrder);
>>>>>>> hx711_implementation
