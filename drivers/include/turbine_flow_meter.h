/*
 * Copyright 2020 Philipp-Alexander Blum
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_turbine_flow_meter Generic driver for turbine flow meters with HAL sensor
 * @ingroup     drivers_sensors
 * @ingroup     drivers_saul
 * @brief       Device driver for turbine flow meter with HAL sensor
 *
 * This driver provides @ref drivers_saul capabilities.
 *
 * @{
 *
 * @file
 * @brief       Device driver interface for turbine flow meter
 *
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 */
#ifndef TURBINE_FLOW_METER_H
#define TURBINE_FLOW_METER_H

#include <stdint.h>

#include "periph/gpio.h"
#include "pulse_counter.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Configuration parameters for the turbine flow meter devices
 */
typedef struct {
    gpio_t pin;                             /**< GPIO pin of the device's data pin */
    uint16_t k_factor;                      /**< The pulses per unit of volume (m^3) */
    float capacity;                         /**< */

} turbine_flow_meter_params_t;

/**
 * @brief   Device descriptor for turbine flow meter devices
 */
typedef struct {
    turbine_flow_meter_params_t params;    /**< Device parameters */
    uint64_t zero_time_usec;               /**< Time of the last zero measurement */
    pulse_counter_t pulse_counter;         /**< Pulse counter, counts raising and falling */
    uint32_t volume_pulse_count;           /**< Pulse count for volume */
    uint32_t flow_pulse_count;             /**< Pulse count for flow*/
} turbine_flow_meter_t;

/**
 * @brief   Initialize a new turbine flow meter device
 *
 * @param[out] dev      device descriptor of a turbine flow meter
 * @param[in]  params   configuration parameters
 *
 * @return              0 on success
 * @return              -1 on error
 */
int turbine_flow_meter_init(turbine_flow_meter_t *dev, const turbine_flow_meter_params_t *params);

/**
 * @brief   Reset a turbine flow meter device
 *
 * @param[out] dev      device descriptor of a turbine flow meter
 * @note                device must be initialized before
 *
 * @return              0 on success
 * @return              -1 on error
 */
int turbine_flow_meter_reset(turbine_flow_meter_t *dev);

/**
 * @brief   get the volume since last volume reset
 *
 * @param[in]  dev  device descriptor of a turbine flow meter
 *
 * @returns volume volume value in m^3 * 10^-1
 */
uint16_t turbine_flow_meter_volume_read(turbine_flow_meter_t *dev);

/**
 * @brief   get the flow value since the last flow reset
 *
 * @param[in]  dev      device descriptor of a turbine flow meter
 *
 * @returns volume volume value in m^3 * 10^-1
 */
uint16_t turbine_flow_meter_flow_read(turbine_flow_meter_t *dev);

    /**
 * @brief   reset the volume counter to a specific value
 *
 * @param[in]  dev  device descriptor of a turbine flow meter
 *
 */
void turbine_flow_meter_volume_reset(turbine_flow_meter_t *dev, uint16_t value);

/**
 * @brief   reset the flow counter to a specific value
 *
 * @param[in]  dev      device descriptor of a turbine flow meter
 * @param[in]  value    raw value of the pulses to reset to
 * @param[in]  time_us  time to reset to in microseconds
 *
 */
void turbine_flow_meter_flow_reset(turbine_flow_meter_t *dev, uint16_t value, uint64_t time_us);

#ifdef __cplusplus
}
#endif

#endif //TURBINE_FLOW_METER_H
/** @} */
