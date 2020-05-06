/*
 * Copyright (C) 2020 Philipp-Alexander Blum <philipp-blum@jakiku.de>
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

#ifndef HX711_H
#define HX711_H

#include "periph/gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @note Read the HX711 datasheet for more information
 */
typedef enum {
    CHANNEL_A_128 = 1,
    CHANNEL_B_32 = 2,
    CHANNEL_A_64 = 3
} hx711_gain_t;

/**
 * @brief   Configuration parameters of the HX711 driver
 * @{
 */
typedef struct {
    gpio_t sck;
    gpio_t dout;
    hx711_gain_t gain;
    uint8_t read_times;
    uint16_t scale;
} hx711_params_t;

typedef struct {
    hx711_params_t params;
    int32_t offset;
} hx711_t;

/** @} */

/**
 * @brief Initializes the hx711 device, without taring it.
 * @param dev The device to initialize
 * @param params The parameters for the hx711 device
 */
void hx711_init(hx711_t *dev, const hx711_params_t *params);

void _hx711_set_gain(hx711_t *dev, hx711_gain_t gain);

/**
 * @brief Gets a single 24-Bit value from the HX711.
 * @return The 24-bit value with gain.
 */
int32_t hx711_read(hx711_t *dev);

/**
 * @brief Read a raw value for a configurable times
 * @param times The amount of repeated measurements. Calculates the average.
 * @return the average value.
 */
int32_t hx711_read_average(hx711_t *dev, uint8_t times);

/**
 * @brief Read a value a configurable times and return the average value.
 * @param times The amount of repeated measurements.
 * @return returns (SUM(RAW_VALUES) / TIMES) - OFFSET
 */
int32_t hx711_get_value(hx711_t *dev, uint8_t times);

/**
 * @brief Read the average of a configurable times of a cleared and scaled value
 * @param times The amount of repeated measurements.
 * @return returns ((SUM(RAW_VALUES) / TIMES) - OFFSET) / SCALE
 */
int32_t hx711_get_units(hx711_t *dev, int8_t times);

/**
 * @brief Set offset for a given hx711 device
 * @param dev The hx711 device
 * @param offset The offset to set on the hx711 device
 */
void _hx711_set_offset(hx711_t *dev, int32_t offset);

/**
 * @brief Get the offset of a given hx711 device
 * @param dev The hx711 device
 * @return returns The offset of the given hx711 device
 */
int32_t _hx711_get_offset(hx711_t *dev);

/**
 * @brief Get the offset and set it to the device. Tare the scale.
 * @param dev The hx711 device
 * @param times The amount of repeated measurements, before setting.
 */
void hx711_tare(hx711_t *dev, uint8_t times);

/**
 * @brief Power a given hx711 down
 * @param dev The hx711 device
 */
void hx711_power_down(hx711_t *dev);

/**
 * @brief Power a given hx711 up
 * @param dev 
 * @return returns ((SUM(RAW_VALUES) / TIMES) - OFFSET) / SCALE
 */
void hx711_power_up(hx711_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* HX711_H */
/** @} */