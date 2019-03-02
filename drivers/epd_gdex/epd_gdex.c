#include <stdint.h>
#include <string.h>
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
uint32_t SLEEP_TIME = 100000;

int epd_gdex_send_command(bool keep_open, epd_gdex_command * command, uint8_t command_length){
    int acquired = spi_acquire(config.SPI_BUS, config.CS_PIN, SPI_MODE, SPI_CLOCK_SPEED);

    if(acquired == 0){
        for(unsigned int i = 0; i < command_length; i++){
            if(command[i].command){
                gpio_set(config.COMMAND_CONTROL_PIN);
            }else{
                gpio_clear(config.COMMAND_CONTROL_PIN);
            }

            spi_transfer_byte(config.SPI_BUS, config.CS_PIN, keep_open, command[i].data);
        }

        if(!keep_open){
            spi_release(config.SPI_BUS);
        }
        return 0;
    }else{
        return acquired;
    }
}

int epd_gdex_send_data(bool keep_open, uint8_t * data, uint8_t data_length){
    gpio_set(config.COMMAND_CONTROL_PIN);

    spi_transfer_bytes(config.SPI_BUS, config.CS_PIN, keep_open, data, NULL, data_length);
    if(!keep_open){
        gpio_clear(config.COMMAND_CONTROL_PIN);
        spi_release(config.SPI_BUS);
    }

    return 0;
}

/*
int epd_gdex_send_command(epd_gdex_command * gdex_command, uint8_t command_length, uint8_t * data, uint8_t data_length){
    int acquired = spi_acquire(config.SPI_BUS, config.CS_PIN, SPI_MODE, SPI_CLOCK_SPEED);

    if(acquired == 0){
        for(unsigned int i = 0; i < command_length; i++){
            if(gdex_command->command){
                gpio_set(config.COMMAND_CONTROL_PIN);
            }else{
                gpio_clear(config.COMMAND_CONTROL_PIN);
            }

            spi_transfer_byte(config.SPI_BUS, config.CS_PIN, true, gdex_command[i].data);
        }

        if(data_length > 0){
            gpio_set(config.COMMAND_CONTROL_PIN);
            spi_transfer_bytes(config.SPI_BUS, config.CS_PIN, false, data, NULL, data_length);
            gpio_clear(config.COMMAND_CONTROL_PIN);
        }

        spi_release(config.SPI_BUS);
        return 0;
    }else{
        return acquired;
    }
}
*/

void wait_until_idle(void){
    while(gpio_read (config.BUSY_PIN) == 0){
        xtimer_usleep(SLEEP_TIME);
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

    epd_gdex_send_command(true, TRES_SETTINGS, sizeof(TRES_SETTINGS) / sizeof(epd_gdex_command));
    epd_gdex_send_data(false, res_data , sizeof(res_data) / sizeof(uint8_t));
}

int epd_gdex_init(epd_gdex_configuration configuration){
    memcpy(&config, &configuration, sizeof(configuration));

    int cs_init = spi_init_cs(config.SPI_BUS, (spi_cs_t) config.CS_PIN);
    if(cs_init != SPI_OK){
        puts("Something went wrong while initializing the SPI bus with the CS PIN");
        return -1;
    }

    int busy_pin_init = gpio_init(config.BUSY_PIN, GPIO_IN_PD);
    int command_pin_init = gpio_init(config.COMMAND_CONTROL_PIN, GPIO_OUT);
    int reset_pin_init = gpio_init(config.RESET_PIN, GPIO_OUT);


    if(busy_pin_init >= 0 || command_pin_init >= 0 || reset_pin_init >= 0){
        return 0;
    }else{
        puts("PIN initialization was not successful! Please check pin configuration.");
        return -1;
    }
}

void epd_gdex_dispay_bootup(void){
    gpio_clear(config.RESET_PIN);
    xtimer_usleep(2* SLEEP_TIME);
    gpio_set(config.RESET_PIN);

    wait_until_idle();

    uint8_t btst_data[] = { 0x17, 0x17, 0x17 };
    epd_gdex_send_command(true, BTST_START, sizeof(BTST_START) / sizeof(epd_gdex_command));
    epd_gdex_send_data(false, btst_data, sizeof(btst_data) / sizeof(uint8_t));

    uint8_t power_settings_data[] = { 0x02, 0x00, 0x2b, 0x2b, 0x09 };
    epd_gdex_send_command(true, POWER_SETTING, sizeof(POWER_SETTING) / sizeof(epd_gdex_command));
    epd_gdex_send_data(false, power_settings_data, sizeof(power_settings_data) / sizeof(uint8_t));

    epd_gdex_send_command(false, POWER_ON, sizeof(POWER_ON) / sizeof(epd_gdex_command));

    wait_until_idle();

    uint8_t panel_settings_data[] = { 0x2F };
    epd_gdex_send_command(true, PANEL_SETTING, sizeof(PANEL_SETTING) / sizeof(epd_gdex_command));
    epd_gdex_send_data(false, panel_settings_data , sizeof(panel_settings_data) / sizeof(uint8_t));

    uint8_t pll_control_data[] = { 0x3a };
    epd_gdex_send_command(true, PLL_CONTROL, sizeof(PLL_CONTROL) / sizeof(epd_gdex_command));
    epd_gdex_send_data(false, pll_control_data , sizeof(pll_control_data) / sizeof(uint8_t));

    //epd_gdex_set_resolution(config.resolution);

    /*uint8_t resolution_data[] = { 0x68, 0x00, 0xD4 };
    epd_gdex_send_command(true, TRES_SETTINGS, sizeof(TRES_SETTINGS) / sizeof(epd_gdex_command));
    epd_gdex_send_data(false, resolution_data , sizeof(resolution_data) / sizeof(char));
    */

    uint8_t vcm_dc_setting_data[] = { 0x12 };
    epd_gdex_send_command(true, VCM_DC_SETTING, sizeof(VCM_DC_SETTING) / sizeof(epd_gdex_command));
    epd_gdex_send_data(false, vcm_dc_setting_data, sizeof(vcm_dc_setting_data) / sizeof(uint8_t));

    uint8_t vcom_data[] = { 0x77 };
    epd_gdex_send_command(
            true, VCOM_AND_DATA_INTERVAL_SETTING, sizeof(VCOM_AND_DATA_INTERVAL_SETTING) / sizeof(epd_gdex_command));
    epd_gdex_send_data(false, vcom_data, sizeof(vcom_data) / sizeof(uint8_t));
}

/*void epd_gdex_display_shutdown(void){

}
 */

void epd_gdex_display_test_frame(void){
    spi_init_pins(config.SPI_BUS);
    xtimer_usleep(SLEEP_TIME);

    epd_gdex_dispay_bootup();

    //96x230
    const unsigned int pixel_count = (96*230) / 8;

    epd_gdex_send_command(true, BLACK_START, sizeof(BLACK_START) / sizeof(epd_gdex_command));
    uint8_t black_pixels[] = { 0x00 };
    for(unsigned int i = 0; i < pixel_count; i++){
        if(i < pixel_count - 1){
            epd_gdex_send_data(true, black_pixels, sizeof(black_pixels) / sizeof(uint8_t));
        }else{
            epd_gdex_send_data(false, black_pixels, sizeof(black_pixels) / sizeof(uint8_t));
        }
    }

    epd_gdex_send_command(true, COLOR_START, sizeof(COLOR_START) / sizeof(epd_gdex_command));
    uint8_t yellow_pixels[] = { 0xFF };
    for(unsigned int i = 0; i < pixel_count; i++){
        if(i < pixel_count - 1){
            epd_gdex_send_data(true, yellow_pixels, sizeof(yellow_pixels) / sizeof(uint8_t));
        }else{
            epd_gdex_send_data(false, yellow_pixels, sizeof(yellow_pixels) / sizeof(uint8_t));
        }
    }

    //epd_gdex_send_command(false, DATA_STOP, sizeof(DATA_STOP) / sizeof(epd_gdex_command));

    epd_gdex_send_command(false, DISPLAY_REFRESH, sizeof(DISPLAY_REFRESH) / sizeof(epd_gdex_command));

    wait_until_idle();

    uint8_t vcom_data[] = { 0x77 };
    epd_gdex_send_command(
            true, VCOM_AND_DATA_INTERVAL_SETTING, sizeof(VCOM_AND_DATA_INTERVAL_SETTING) / sizeof(epd_gdex_command));
    epd_gdex_send_data(false, vcom_data, sizeof(vcom_data) / sizeof(char));

    epd_gdex_send_command(false, POWER_OFF, sizeof(POWER_OFF) / sizeof(epd_gdex_command));

    epd_gdex_send_command(false, DEEP_SLEEP, sizeof(DEEP_SLEEP) / sizeof(epd_gdex_command));
}

/*void epd_gdex_display_frame(unsigned char * frame_black, unsigned char * frame_red){

}*/
