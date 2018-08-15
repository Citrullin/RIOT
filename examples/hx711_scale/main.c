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
#include "bitarithm.h"
#include "xtimer.h"

typedef enum {
    MSBFIRST,
    LSBFIRST
} bitarithm_bit_order_t;

uint8_t bitarithm_shift_in(gpio_t dataPin, gpio_t clockPin, bitarithm_bit_order_t bitOrder){
    uint8_t byte = 0x00;

    for(int i = 0; i <= 8; i++){
        gpio_set(clockPin);
        if(gpio_read(dataPin) > 0){
            if(bitOrder == MSBFIRST){
                byte = byte | 0x01 << (8 - i);
            }
            else if(bitOrder == LSBFIRST){
                byte = byte | 0x01 << i;
            } else {
                return '\0';
            }
        }
        gpio_clear(clockPin);
    }

    return byte;
}


int PD_SCK = GPIO_PIN(1, 10);
int DOUT = GPIO_PIN(1, 11);
uint8_t GAIN = 1;		// amplification factor
uint8_t OFFSET = 0;	// used for tare weight
float SCALE = 1; // used to return weight in grams, kg, ounces, whatever

long hx711_read(void){
    // wait for the chip to become ready
    /*while (!hx711_is_ready()) {
        // Will do nothing on Arduino but prevent resets of ESP8266 (Watchdog Issue)
        yield();
    }*/

    unsigned long value = 0;
    uint8_t data[3] = { 0 };
    uint8_t filler = 0x00;

    // pulse the clock pin 24 times to read the data
    data[2] = bitarithm_shift_in(DOUT, PD_SCK, MSBFIRST);
    data[1] = bitarithm_shift_in(DOUT, PD_SCK, MSBFIRST);
    data[0] = bitarithm_shift_in(DOUT, PD_SCK, MSBFIRST);

    // set the channel and the gain factor for the next reading using the clock pin
    for (unsigned int i = 0; i < GAIN; i++) {
        gpio_set(PD_SCK);
        gpio_clear(PD_SCK);
    }

    // Replicate the most significant bit to pad out a 32-bit signed integer
    if (data[2] & 0x80) {
        filler = 0xFF;
    } else {
        filler = 0x00;
    }

    // Construct a 32-bit signed integer
    value = ( (unsigned long)(filler) << 24
              | (unsigned long)(data[2]) << 16
              | (unsigned long)(data[1]) << 8
              | (unsigned long)(data[0]) );

    return (long)(value);
}


void hx711_set_gain(uint8_t gain){
    switch (gain) {
        case 128:		// channel A, gain factor 128
            GAIN = 1;
            break;
        case 64:		// channel A, gain factor 64
            GAIN = 3;
            break;
        case 32:		// channel B, gain factor 32
            GAIN = 2;
            break;
    }

    gpio_clear(PD_SCK);

    hx711_read();

}

void hx711_init(uint8_t gain){
    gpio_init(PD_SCK, GPIO_OUT);
    gpio_init(DOUT, GPIO_IN);

    hx711_set_gain(gain);
}

uint32_t hx711_read_average(uint32_t times){
    uint32_t sum = 0;
    for (uint8_t i = 0; i < times; i++) {
        sum += hx711_read();
        //yield();
    }
    return sum / times;
}


double hx711_get_value(uint8_t times) {
    return hx711_read_average(times) - OFFSET;
}

float hx711_get_units(uint8_t times) {
    return hx711_get_value(times) / SCALE;
}

void hx711_set_offset(long offset) {
    OFFSET = offset;
}

void hx711_tare(uint8_t times) {
    double sum = hx711_read_average(times);
    hx711_set_offset(sum);
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

void hx711_power_down(void) {
    gpio_clear(PD_SCK);
    gpio_set(PD_SCK);
}

void hx711_power_up(void) {
    gpio_clear(PD_SCK);
}

int main(void)
{
    puts("RIOT HX711 Scale");
    puts("=====================================");

    puts("Init HX711...");
    hx711_init(128);
    puts("Initialized HX711.");
    printf("Before setup: %lu\n", hx711_read());


    printf("Read average: %lu\n", hx711_read_average(5));

    printf("Get Units: %f\n", hx711_get_units(5));

    hx711_set_scale(2280.f);

    hx711_tare(10);

    while(true){
        printf("Read value: %f\n", hx711_get_units(10));

        hx711_power_down();
        xtimer_sleep(2);
        hx711_power_up();
    }


    return 0;
}
