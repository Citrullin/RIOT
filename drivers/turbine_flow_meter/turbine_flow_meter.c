/*
 * Copyright (C) 2020 Philipp-Alexander Blum
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_turbine_flow_meter
 * @{
 *
 * @file
 * @brief       Generic device driver implementation for turbine flow meters
 *
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * @}
 */

#include <stdint.h>
#include <string.h>

#include "log.h"
#include "assert.h"
#include "pulse_counter.h"
#include "xtimer.h"
#include "timex.h"
#include "periph/gpio.h"

#include "turbine_flow_meter.h"
#include "turbine_flow_meter_params.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"

int turbine_flow_meter_init(turbine_flow_meter_t *dev, const turbine_flow_meter_params_t *params)
{
    assert(dev && params);
    dev->params = *params;

    dev->zero_time_usec = xtimer_now_usec64();
    pulse_counter_params_t pulse_counter_params = {
            .gpio = dev->params.pin,
            .gpio_flank = GPIO_BOTH,
    };

    if(pulse_counter_init(&dev->pulse_counter, &pulse_counter_params) == -1) {
        puts("Error while initializing flow_meter");
        return 1;
    }

    return 0;
}

void turbine_flow_meter_volume_reset(turbine_flow_meter_t *dev, uint16_t value)
{
    dev->volume_pulse_count = value;
}

void turbine_flow_meter_flow_reset(turbine_flow_meter_t *dev, uint16_t value, uint64_t time_us)
{
    dev->zero_time_usec = time_us;
    dev->flow_pulse_count = value;
}

uint16_t turbine_flow_meter_volume_read(turbine_flow_meter_t *dev)
{
    uint16_t pulses = pulse_counter_read_with_reset(&dev->pulse_counter) / 2;
    dev->volume_pulse_count += pulses;
    dev->flow_pulse_count += pulses;

    printf("Pulses: %li\n", dev->volume_pulse_count);
    return (uint16_t) (dev->volume_pulse_count / dev->params.k_factor);
}

uint16_t turbine_flow_meter_flow_read(turbine_flow_meter_t *dev)
{
    uint16_t pulses = pulse_counter_read_with_reset(&dev->pulse_counter) / 2;
    dev->volume_pulse_count += pulses;
    dev->flow_pulse_count += pulses;

    printf("Flow result: %li \n", dev->flow_pulse_count / dev->params.k_factor);

    return (uint16_t) (
            (dev->flow_pulse_count / dev->params.k_factor) /
            ( (xtimer_now_usec64() - dev->zero_time_usec) / 1000000 )
    );
}