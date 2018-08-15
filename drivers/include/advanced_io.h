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

#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t advanced_io_shift_in(gpio_t dataPin, gpio_t clockPin);

#ifdef __cplusplus
}
#endif
