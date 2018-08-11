typedef enum {
    MSBFIRST,
    LSBFIRST
} advanced_io_bit_order_t;

uint8_t advanced_io_shift_in(gpio_t dataPin, gpio_t clockPin, advanced_io_bit_order_t bitOrder);