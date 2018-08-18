/*
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example demonstrating of a IOTA address generation in RIOT
 *
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * @}
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "thread.h"
#include "periph/gpio.h"
#include "hx711.h"

void calibrate(void){
    hx711_set_offset(0.f);
    hx711_set_scale(1.f);

    hx711_power_up();

    puts("Calibration...");
    float f = hx711_get_units(50);
    printf("VALUE: %i \n",(int) f);
    hx711_set_offset(f - 10000);
    hx711_set_scale(0.333);

    hx711_power_down();
    puts("Calibrated.");
}

int main(void)
{
    puts("RIOT HX711 Scale");
    puts("=====================================");

    puts("Init HX711...");

    hx711_init(128, (int) GPIO_PIN(1, 13), (int) GPIO_PIN(1, 14));
    puts("Initialized HX711.");

    calibrate();

    while(true){
        hx711_power_up();
        printf("Read value: %i\n", (int) hx711_get_units(10));
        hx711_power_down();
    }


    return 0;
}
