#include <stdint.h>
#include "periph/gpio.h"
#include "periph/spi.h"
#include "epd_gdex_commands.c"

typedef struct {
    uint8_t SPI_BUS = 1;
    uint8_t CS_PIN = GPIO_PIN(0, 0);

    uint8_t BUS_SELECTION_PIN = GPIO_PIN(0, 0);
    uint8_t COMMAND_CONTROL_PIN = GPIO_PIN(0, 0);
    uint8_t RESET_PIN = GPIO_PIN(0, 0);
    uint8_t BUSY_PIN = GPIO_PIN(0, 0);

    epd_gdex_resolution resolution = res_96x230;
    epd_gdex_display_color_mode = mode_color;
    epd_gdex_frame_rate = rate_50hz;

    epd_gdex_lut lut = lut_otp;
    epd_gdex_booster_switch = booster_on;

    epd_gdex_scan_direction = scan_up;
    epd_gdex_shift_direction = shift_right;

    epd_gdex_source_power source_power = external_source_power;
    epd_gdex_gate_source = external_gate_power;

    epd_gdex_voltage_level voltage_level = level_16v;
    epd_gdex_power_selection power_selection = power_10_0v;
} epd_gdex_configuration;

epd_gdex_configuration config = {};
spi_mode_t SPI_MODE = SPI_MODE_0;
spi_clk_t SPI_CLOCK_SPEED = SPI_CLK_1MHZ;

void epd_gdex_init(epd_gdex_configuration configuration){
    config = configuration;
    spi_init_pins(config.SPI_BUS);
}

void epd_gdex_power_on(){
    int result = spi_acquire(config.SPI_BUS, config.CS_PIN, SPI_MODE, SPI_CLOCK_SPEED);
}
