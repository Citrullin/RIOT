/*
 * Copyright (C) 2019 Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_hx711 HX711 digital scale sensor
 * @ingroup     drivers_sensors
 * @brief       Driver for the HX711 digital scale sensor
 *
 * @{
 * @file
 * @brief       HX711 driver
 *
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 */

void hx711_init(void);

/**
 * @brief Read the average of a configurable times of a cleared and scaled value. Always rounded up.
 * @param times
 * @return returns (AVG(RAW_VALUE) - HX711_PARAM_OFFSET) / HX711_PARAM_SCALE
 */
int32_t hx711_get_units(int8_t times);

void hx711_power_down(void);

void hx711_power_up(void);
