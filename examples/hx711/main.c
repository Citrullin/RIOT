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
#include "periph/gpio_util.h"
#include "periph/gpio.h"
#include "bitarithm.h"
#include "xtimer.h"

typedef enum {
    CHANNEL_A_128,
    CHANNEL_B_32,
    CHANNEL_A_64,
} HX711_CHANNEL_GAIN;

gpio_t PD_SCK = GPIO_PIN(1, 13);
gpio_t DOUT = GPIO_PIN(1, 14);
HX711_CHANNEL_GAIN CHANNEL_GAIN = CHANNEL_A_128;	// amplification factor
int32_t OFFSET = 9000;	// used for tare weight
float SCALE = 1; // used to return weight in grams, kg, ounces, whatever

uint32_t SLEEP_TIME = 1000;


void wait_for_ready(void){
    while(gpio_read(DOUT) > 0){
        xtimer_usleep(SLEEP_TIME);
    }
}

unsigned int hx711_get_extra_pulses(HX711_CHANNEL_GAIN channel_gain){
    switch(channel_gain){
        case CHANNEL_A_128:
            return 1;
        case CHANNEL_B_32:
            return 2;
        case CHANNEL_A_64:
            return 3;
        default:
            return 1;
    }
}

int32_t hx711_read(void){
    wait_for_ready();

    uint8_t data[3];
    uint32_t filler = 0x00;

    // pulse the clock pin 24 times to read the data
    for(int i = 2; i >= 0; i--){
        uint8_t current_data = gpio_util_shiftin(DOUT, PD_SCK);
        data[i] = current_data;
    }

    // set the channel and the gain factor for the next reading using the clock pin
    unsigned int extra_pulses = hx711_get_extra_pulses(CHANNEL_GAIN);
    for (unsigned int i = 0; i < extra_pulses; i++) {
        gpio_set(PD_SCK);
        gpio_clear(PD_SCK);
    }

    if (data[2] & 0x80) {
        filler = 0xFF;
    } else {
        filler = 0x00;
    }

    return (int32_t) ((filler) << 24 | data[2] << 16 | data[1] << 8| data[0]);
}


void hx711_set_gain(HX711_CHANNEL_GAIN channel_gain){
    CHANNEL_GAIN = channel_gain;
}

uint32_t hx711_read_average(uint32_t times){
    uint32_t sum = 0;
    for (uint8_t i = 0; i < times; i++) {
        sum += hx711_read();
    }
    return sum / times;
}

void hx711_init(HX711_CHANNEL_GAIN channel_gain){
    gpio_init(PD_SCK, GPIO_OUT);
    gpio_init(DOUT, GPIO_IN);

    hx711_set_gain(channel_gain);
}

void hx711_power_down(void){
    gpio_set(PD_SCK);
    xtimer_usleep(100);
}

void hx711_power_up(void){
    gpio_clear(PD_SCK);
}

int32_t hx711_get_value(uint8_t times) {
    int32_t value = (hx711_read_average(times) - OFFSET);

    if(value < 0){
        return 0;
    }else{
        return value;
    }
}

double hx711_get_units(uint8_t times) {
    return (hx711_get_value(times) - OFFSET) / SCALE;
}

void hx711_set_offset(uint32_t offset) {
    OFFSET = offset;
}

void hx711_tare(uint8_t times) {
    uint32_t offset = hx711_read_average(times);
    hx711_set_offset(offset);
}

void hx711_set_scale(float scale) {
    SCALE = scale;
}

float hx711_get_scale(void) {
    return SCALE;
}

long hx711_get_offset(void) {
    return OFFSET;
}

int main(void)
{
    puts("RIOT HX711 Scale");
    puts("=====================================");

    puts("Init HX711...");
    //hx711_power_down();
    hx711_init(CHANNEL_A_128);
    hx711_power_up();
    puts("Initialized HX711.");

    //hx711_set_scale(2280.f);

    hx711_tare(5);

    while(true){
        printf("Read value: %li\n", hx711_get_value(10));
        printf("Read single value: %lu\n", hx711_read());

        xtimer_sleep(2);
    }


    return 0;
}
