#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "periph/gpio.h"
#include "epd_gdex.h"

#include "xtimer.h"

int main(void)
{
    puts("ePaper test");
    puts("=====================================");

    /*
    gpio_t pin = GPIO_PIN(0, 9);
    //gpio_t cs_pin = SPI_HWCS(2);

    int init = gpio_init(pin, GPIO_OUT);

    printf("INIT: %i\n", init);

    xtimer_sleep(1);

    gpio_clear(pin);

    xtimer_sleep(3);

    gpio_set(pin);

    xtimer_sleep(3);

    gpio_clear(pin);

     */



    gpio_t control_pin = GPIO_PIN(0, 10);
    gpio_t reset_pin = GPIO_PIN(0, 9);
    gpio_t busy_pin = GPIO_PIN(0, 8);
    //gpio_t cs_pin = SPI_HWCS(1);
    gpio_t cs_pin = GPIO_PIN(1, 12);

    epd_gdex_configuration epd_config = {
            .SPI_BUS = 1,
            .CS_PIN = cs_pin,
            .COMMAND_CONTROL_PIN = control_pin,
            .RESET_PIN = reset_pin,
            .BUSY_PIN = busy_pin
    };

    xtimer_sleep(3);
    epd_gdex_init(epd_config);

    epd_gdex_display_test_frame();


    return 0;
}