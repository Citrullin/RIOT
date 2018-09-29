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
 * @brief       Example demonstrating of transaction in IOTA with RIOT
 *https://github.com/embedded-iota/iota-c-light-wallet
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * @}
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "ble-core.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "shell.h"

#define LED_PIN            GPIO_PIN(0, 4)

//Fixme: It is not preparing a tx bundle
int main(void)
{
    puts("IOTA Wallet Application");
    puts("=====================================");

    ble_stack_init();
    ble_advertising_init("RIOTOS");
    ble_advertising_start();

    /*
    gpio_init(LED_PIN, GPIO_OUT);

    xtimer_init();
    while(1){
        gpio_clear(LED_PIN);
        xtimer_sleep(2);
        puts("test");
        gpio_set(LED_PIN);
    }*/


    return 0;
}
