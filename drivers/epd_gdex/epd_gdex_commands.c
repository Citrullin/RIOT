#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool command;
    bool read;
    uint8_t data;
} epd_gdex_command;

epd_gdex_command POWER_SETTING[1] = {
        { .command = false, .read = false, .data = 0x00 }
};

epd_gdex_command POWER_ON[1] = {
        { .command = false, .read = false, .data = 0x04 }
};

epd_gdex_command POWER_OFF[1] = {
        { .command = false, .read = false, .data = 0x02 }
};

epd_gdex_command DEEP_SLEEP[2] = {
        { .command = false, .read = false, .data = 0x07 },
        { .command = false, .read = false, .data = 0xA5 }
};

epd_gdex_command DISPLAY_REFRESH[1] = {
        { .command = false, .read = false, .data = 0x12 }
};

/*
 * BTST = Booster Soft Start
 */
epd_gdex_command BTST_START[1] = {
        { .command = false, .read = false, .data = 0x06 }
};

epd_gdex_command BLACK_START[1] = {
        { .command = false, .read = false, .data = 0x10 }
};

epd_gdex_command COLOR_START[1] = {
        { .command = false, .read = false, .data = 0x13 }
};

epd_gdex_command DATA_STOP[2] = {
        { .command = false, .read = false, .data = 0x11 }
};

epd_gdex_command TEMPERATURE_SENSOR_CALIBRATION_START[1] = {
        { .command = false, .read = false, .data = 0x40 }
};

epd_gdex_command TEMPERATURE_SENSOR_SELECTION_START[1] = {
        { .command = false, .read = false, .data = 0x41 }
};

epd_gdex_command VCOM_AND_DATA_INTERVAL_SETTING[1] = {
        { .command = false, .read = false, .data = 0x50 }
};

epd_gdex_command TRES_SETTINGS[1] = {
        { .command = false, .read = false, .data = 0x61 }
};

epd_gdex_command VCM_DC_SETTING[1] = {
        { .command = false, .read = false, .data = 0x82 }
};