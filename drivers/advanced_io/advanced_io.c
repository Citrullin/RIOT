#include "periph/gpio.h"

typedef enum {
    MSBFIRST,
    LSBFIRST
} advanced_io_bit_order_t;

uint8_t advanced_io_shift_in(gpio_t dataPin, gpio_t clockPin, advanced_io_bit_order_t bitOrder){
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
