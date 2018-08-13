/*
 * Copyright (C) 2017 Philipp-Alexander Blum <philipp-blum@jakiku.de>
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

void hx711_set_gain(uint8_t gain);

void hx711_init(uint8_t gain, int sck, int dout);

uint32_t hx711_read_average(uint32_t times);

double hx711_get_value(uint8_t times);

float hx711_get_units(uint8_t times);

void hx711_set_offset(long offset);

void hx711_tare(uint8_t times);

void hx711_set_scale(float scale);

float hx711_get_scale(void);

long hx711_get_offset(void);

void hx711_power_down(void);

void hx711_power_up(void);
