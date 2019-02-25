typedef struct {
    bool command;
    bool read;
    uint8_t data;
} epd_gdex_command;

typedef enum {
    res_96x230,
    res_96x252,
    res_128x296,
    res_160x296
} epd_gdex_resolution;

typedef enum {
    lut_otp,
    lut_register
} epd_gdex_lut;

typedef enum {
    mode_black_white,
    mode_color
} epd_gdex_display_color_mode;

typedef enum {
    scan_up,
    scan_down
} epd_gdex_scan_direction;

typedef enum {
    shift_left,
    shift_right
} epd_gdex_shift_direction;

typedef enum {
    booster_on,
    booster_off
} epd_gdex_booster_switch;

typedef enum {
    external_source_power,
    internal_source_power
} epd_gdex_source_power;

typedef enum {

} epd_gdex_source_power;

typedef enum {
    external_gate_power,
    internal_gate_power
} epd_gdex_gate_source;

typedef enum {
    rate_4hz,
    rate_5hz,
    rate_6hz,
    rate_7hz,
    rate_8hz,
    rate_10hz,
    rate_11hz,
    rate_12hz,
    rate_14hz,
    rate_16hz,
    rate_17hz,
    rate_19hz,
    rate_20hz,
    rate_21hz,
    rate_23hz,
    rate_24hz,
    rate_29hz,
    rate_34hz,
    rate_36hz,
    rate_38hz,
    rate_40hz,
    rate_43hz,
    rate_48hz,
    rate_50hz,
    rate_57hz,
    rate_67hz,
    rate_72hz,
    rate_86hz,
    rate_100hz,
    rate_114hz,
    rate_150hz,
    rate_171hz
} epd_gdex_frame_rate;

typedef enum {
    level_13v,
    level_14v,
    level_15v,
    level_16v
} epd_gdex_voltage_level;

typedef enum {
    power_2_4v,
    power_2_6v,
    power_2_6v,
    power_3_0v,
    power_3_2v,
    power_3_4v,
    power_3_6v,
    power_3_8v,
    power_4_0v,
    power_4_2v,
    power_4_4v,
    power_4_6v,
    power_4_8v,
    power_5_0v,
    power_5_2v,
    power_5_4v,
    power_5_6v,
    power_5_8v,
    power_6_0v,
    power_6_2v,
    power_6_4v,
    power_6_6v,
    power_6_8v,
    power_7_0v,
    power_7_2v,
    power_7_4v,
    power_7_6v,
    power_7_8v,
    power_8_0v,
    power_8_2v,
    power_8_4v,
    power_8_6v,
    power_8_8v,
    power_9_0v,
    power_9_2v,
    power_9_4v,
    power_9_6v,
    power_9_8v,
    power_10_0v,
    power_10_2v,
    power_10_4v,
    power_10_6v,
    power_10_8v,
    power_11_0v
} epd_gdex_power_selection;


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

epd_gdex_command WHITE_BLACK_START[1] = {
        { .command = false, .read = false, .data = 0x10 }
};

epd_gdex_command SET_COLOR_START[1] = {
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