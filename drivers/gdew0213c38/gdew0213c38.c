#include <stdint.h>
#include "periph/gpio.h"
#include "periph/spi.h"

uint8_t GDEW0213_SPI_SCK = GPIO_PIN(0, 0);
uint8_t GDEW_SPI_DOUT = GPIO_PIN(0, 0);
uint8_t GDEW_COMMAND_CONTROL_PIN = GPIO_PIN(0, 0);
uint8_t GDEW_RESET_PIN = GPIO_PIN(0, 0);