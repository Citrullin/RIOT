#include <stdint.h>
#include "periph/gpio.h"
#include "periph/spi.h"
#include "xtimer.h"

#include "include/epd_gdex.h"
#include "epd_gdex_commands.c"

/*typedef struct {
    uint8_t SPI_BUS = 1;
    uint8_t CS_PIN = GPIO_PIN(0, 0);

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
*/

epd_gdex_configuration config = {};
spi_mode_t SPI_MODE = SPI_MODE_0;
spi_clk_t SPI_CLOCK_SPEED = SPI_CLK_1MHZ;

int epd_gdex_send_command(epd_gdex_command * gdex_command, uint8_t command_length, uint8_t * data, uint8_t data_length){
    int acquired = spi_acquire(config.SPI_BUS, config.CS_PIN, SPI_MODE, SPI_CLOCK_SPEED);

    if(acquired == 0){
        for(unsigned int i = 0; i < command_length; i++){
            if(gdex_command->command){
                gpio_set(config.COMMAND_CONTROL_PIN);
            }else{
                gpio_clear(config.COMMAND_CONTROL_PIN);
            }

            spi_transfer_byte(config.SPI_BUS, config.CS_PIN, true,gdex_command->data);
        }

        if(data_length > 0){
            gpio_set(config.COMMAND_CONTROL_PIN);
            spi_transfer_bytes(config.SPI_BUS, config.CS_PIN, false, data, NULL, data_length);
            gpio_clear(config.COMMAND_CONTROL_PIN);
        }

        return 0;
    }else{
        return acquired;
    }
}

void wait_until_idle(void){
    while(gpio_read (config.BUSY_PIN) == 0){
        xtimer_usleep(100000);
    }
}

uint8_t epd_gdex_get_v_resolution(epd_gdex_resolution resolution){
    switch(resolution){
        case res_96x230:
            return 96;
        case res_96x252:
            return 96;
        case res_128x296:
            return 128;
        case res_160x296:
            return 160;
        default:
            return 96;
    }
}

uint16_t epd_gdex_get_h_resolution(epd_gdex_resolution resolution){
    switch(resolution){
        case res_96x230:
            return 230;
        case res_96x252:
            return 252;
        case res_128x296:
            return 296;
        case res_160x296:
            return 296;
        default:
            return 230;
    }
}

void epd_gdex_set_resolution(epd_gdex_resolution resolution){
    uint8_t v_res_data = (uint8_t) epd_gdex_get_v_resolution(resolution) >> 3;
    uint8_t h_res_data_upper = (uint8_t)(epd_gdex_get_h_resolution(resolution) >> 8);
    uint8_t h_res_data_lower = (uint8_t) epd_gdex_get_h_resolution(resolution);

    uint8_t res_data[] = { v_res_data, h_res_data_upper, h_res_data_lower };

    epd_gdex_send_command(TRES_SETTINGS, sizeof(TRES_SETTINGS), res_data , sizeof(res_data));
}

int epd_gdex_init(epd_gdex_configuration configuration){
    config = configuration;

    spi_init(config.SPI_BUS);
    uint8_t btst_data[] = { 0x17, 0x17, 0x17 };
    epd_gdex_send_command(BTST_START, sizeof(BTST_START), btst_data, sizeof(btst_data));
    epd_gdex_send_command(POWER_ON, sizeof(POWER_ON), NULL, 0);
    wait_until_idle();
    uint8_t settings_data[] = { 0x8F };
    epd_gdex_send_command(POWER_SETTING, sizeof(POWER_SETTING), settings_data , sizeof(settings_data));
    uint8_t vcom_data[] = { 0x77 };
    epd_gdex_send_command(VCOM_AND_DATA_INTERVAL_SETTING, sizeof(VCOM_AND_DATA_INTERVAL_SETTING), vcom_data, sizeof(vcom_data));
    epd_gdex_set_resolution(config.resolution);
    //epd_gdex_send_command(VCM_DC_SETTING, { 0X0A }, 1);

    return 0;
}

void epd_gdex_display_test_frame(void){
    uint8_t black_data[] = { 0x05, 0xA7, 0x8A };
    epd_gdex_send_command(BLACK_START, sizeof(BLACK_START), black_data, sizeof(black_data));
    uint8_t color_data[] = { 0x05, 0xA7, 0x8A };
    epd_gdex_send_command(COLOR_START, sizeof(COLOR_START), color_data, sizeof(color_data));
    uint8_t stop_data[] = { 0x01 };
    epd_gdex_send_command(DATA_STOP, sizeof(DATA_STOP), stop_data, sizeof(stop_data));
}

/*void epd_gdex_display_frame(unsigned char * frame_black, unsigned char * frame_red){

}*/
