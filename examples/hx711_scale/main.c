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


int main(void)
{
    puts("RIOT HX711 Scale");
    puts("=====================================");

    puts("Init HX711...");

    hx711_init(128, (int) GPIO_PIN(1, 13), (int) GPIO_PIN(1, 14));
    puts("Initialized HX711.");

    hx711_set_offset(197485.f);
    hx711_set_scale(1.455);

    while(true){
        hx711_power_up();
        printf("Read value: %i\n", (int) hx711_get_units(10));
        hx711_power_down();
    }


    return 0;
}
