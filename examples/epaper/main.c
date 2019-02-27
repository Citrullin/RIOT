#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "periph/gpio.h"
#include "epd_gdex.h"

int main(void)
{
    puts("ePaper test");
    puts("=====================================");

    gpio_t cs_pin = GPIO_PIN(0, 15);
    gpio_t control_pin = GPIO_PIN(0, 11);
    gpio_t reset_pin = GPIO_PIN(0, 12);
    gpio_t busy_pin = GPIO_PIN(0, 10);

    epd_gdex_configuration epd_config = {
            .SPI_BUS = 1,
            .CS_PIN = cs_pin,
            .COMMAND_CONTROL_PIN = control_pin,
            .RESET_PIN = reset_pin,
            .BUSY_PIN = busy_pin
    };

    epd_gdex_init(epd_config);

    epd_gdex_display_test_frame();

    return 0;
}