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

#include "periph/gpio.h"
#include "advanced_io.h"

int HX711_SCK = GPIO_PIN(1, 13);
int HX711_DOUT =  GPIO_PIN(1, 14);
uint8_t HX711_GAIN = 1;
float HX711_OFFSET = 0.0f;
float HX711_SCALE = 1.0f;

void yield(void) {};

bool hx711_is_ready(void) {
    return gpio_read(HX711_DOUT) == 0;
}

float hx711_read(void){
    while (!hx711_is_ready()) {
        yield();
    }

    float value = 0;
    uint8_t data[3];
    uint8_t fill_byte = 0x00;

    data[2] = advanced_io_shift_in(HX711_DOUT, HX711_SCK, MSBFIRST);
    data[1] = advanced_io_shift_in(HX711_DOUT, HX711_SCK, MSBFIRST);
    data[0] = advanced_io_shift_in(HX711_DOUT, HX711_SCK, MSBFIRST);


    for (int i = 0; i < HX711_GAIN; i++) {
        gpio_set(HX711_SCK);
        gpio_clear(HX711_SCK);
    }


    if (data[2] & 0x80) {
        fill_byte = 0xFF;
    } else {
        fill_byte = 0x00;
    }

    value = ( (unsigned long)(fill_byte) << 24
              | (unsigned long)(data[2]) << 16
              | (unsigned long)(data[1]) << 8
              | (unsigned long)(data[0]) );

    return (float)(value);
}


void hx711_set_gain(uint8_t gain){
    switch (gain) {
        case 128:
            HX711_GAIN = 1;
            break;
        case 64:
            HX711_GAIN = 3;
            break;
        case 32:
            HX711_GAIN = 2;
            break;
    }

    gpio_clear(HX711_SCK);
    hx711_read();
}

void hx711_set_sck(int sck){
    HX711_SCK = sck;
}

void hx711_set_dout(int dout){
    HX711_DOUT = dout;
}

void hx711_init(uint8_t gain, int sck, int dout){
    hx711_set_sck(sck);
    hx711_set_dout(dout);

    gpio_init(HX711_SCK, GPIO_OUT);
    gpio_init(HX711_DOUT, GPIO_IN);

    hx711_set_gain(gain);
}

uint32_t hx711_read_average(uint32_t times){
    uint32_t sum = 0;
    for (uint8_t i = 0; i < times; i++) {
        sum += hx711_read();
        yield();
    }
    return sum / times;
}


double hx711_get_value(uint8_t times) {
    return hx711_read_average(times) - HX711_OFFSET;
}

float hx711_get_units(uint8_t times) {
    return hx711_get_value(times) / HX711_SCALE;
}

void hx711_set_offset(long offset) {
    HX711_OFFSET = offset;
}

void hx711_tare(uint8_t times) {
    double sum = hx711_read_average(times);
    hx711_set_offset(sum);
}

void hx711_set_scale(float scale) {
    HX711_SCALE = scale;
}

float hx711_get_scale(void) {
    return HX711_SCALE;
}

long hx711_get_offset(void) {
    return HX711_OFFSET;
}

void hx711_power_down(void) {
    gpio_clear(HX711_SCK);
    gpio_set(HX711_SCK);
}

void hx711_power_up(void) {
    gpio_clear(HX711_SCK);
}
