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

#include "iota/kerl.h"
#include "iota/conversion.h"
#include "iota/addresses.h"

#include "periph/uart.h"
#include "periph/gpio.h"

#include "xtimer.h"


int UART_PORT = UART_DEV(2);

int M0_PIN = GPIO_PIN(1, 1);
int M1_PIN = GPIO_PIN(1, 0);

int AUX_PIN = GPIO_PIN(1, 8);


typedef struct {
    char data[100];
    uint8_t length;
} callback_buffer_t;

callback_buffer_t buffer = {};


void uart_callback(void *args, uint8_t data){
    (void) args;

    buffer.data[buffer.length] = data;
    buffer.length = buffer.length + 1;
}

uart_rx_cb_t callback_ptr = (uart_rx_cb_t) &uart_callback;

int setup(void){
    gpio_clear(M0_PIN);
    gpio_clear(M1_PIN);

    gpio_init(M0_PIN, GPIO_OUT);
    gpio_init(M1_PIN, GPIO_OUT);


    return 0;
}

uint8_t * SUBSCRIBE_COMMAND = (uint8_t *) 0x01;


char charAddress[82];
unsigned char address[81];
unsigned char seedBytes[48];

uint8_t package_buffer[51];

int clear_package_buffer(void){
    for(unsigned int i = 0; i < 51; i++){
        package_buffer[i] = 0x00;
    }

    return 0;
}

int subscribe_address(void){
    printf("\n%s\n", charAddress);
    memcpy(package_buffer, charAddress, 50);

    printf("\nWrite: %s\n", package_buffer);
    uart_write(UART_PORT, (uint8_t *) package_buffer, 50);
    xtimer_usleep(3000);

    unsigned int rest_size = 81 - 50;
    clear_package_buffer();
    memcpy(package_buffer, charAddress + 57, rest_size);

    printf("\nWrite: %s\n", package_buffer);
    uart_write(UART_PORT, (uint8_t *) package_buffer, rest_size);
    xtimer_usleep(3000);


    return 0;
}

char seedChars[] = "YOURSEED";
uint32_t address_index = 0;

int init_seed(void){
    chars_to_bytes(seedChars, seedBytes, 81);
    return 0;
}

int generate_address(void){

    get_public_addr(seedBytes, address_index, 2, address);


    bytes_to_chars(address, charAddress, 48);

    printf("Generate IOTA address %li ...\n", address_index);
    printf("Generated address: ");

    charAddress[81] = '\0';
    printf("%s\n", charAddress);
    puts("Generation done.");

    address_index++;

    return 0;
}

int main(void)
{
    puts("IOTA Wallet Application");
    puts("=====================================");

    setup();

    puts("Init UART...");
    int result = uart_init(UART_PORT, 9600, callback_ptr, &buffer);

    if(result == 0){
        puts("Init of UART was successful.");
    }else{
        puts("An error accrued while init UART.");
    }

    uart_poweron(UART_PORT);

    init_seed();
    while(true){
        generate_address();
        puts("Subscribe with address via Lora...");
        subscribe_address();
        puts("Subscription message sent.");
        xtimer_usleep(5000000);
    }


    return 0;
}
