/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include "periph/i2c.h"

#define SDA GPIO(0, 0)
#define SCL GPIO(0, 0)

//Is triggered, when a touch or release is detected
#define IRQ GPIO(0, 0)

#define MPR121_ELECTRODE_CONF_REG 0x5E

#define MPR121_ELE0_TO_ELE7_TOUCH_STATUS_REG 0x00
#define MPR121_ELE8_TO_ELE11_ELEPROX_TOUCH_STATUS_REG 0x01

#define MPR121_ELE0_TO_ELE7_OOR_STATUS_REG 0x02
#define MPR121_ELE7_TO_ELE11_ELEPROX_OOR_STATUS_REG 0x03

#define MPR121_ELE0_FILTERED_LSB_REG 0x04
#define MPR121_ELE0_FILTERED_MSB_REG 0x05
#define MPR121_ELE1_FILTERED_LSB_REG 0x06
#define MPR121_ELE1_FILTERED_MSB_REG 0x07
#define MPR121_ELE2_FILTERED_LSB_REG 0x08
#define MPR121_ELE2_FILTERED_MSB_REG 0x09
#define MPR121_ELE3_FILTERED_LSB_REG 0x0A
#define MPR121_ELE3_FILTERED_MSB_REG 0x0B
#define MPR121_ELE4_FILTERED_LSB_REG 0x0C
#define MPR121_ELE4_FILTERED_MSB_REG 0x0D
#define MPR121_ELE5_FILTERED_LSB_REG 0x0E
#define MPR121_ELE5_FILTERED_MSB_REG 0x0F
#define MPR121_ELE6_FILTERED_LSB_REG 0x10
#define MPR121_ELE6_FILTERED_MSB_REG 0x11
#define MPR121_ELE7_FILTERED_LSB_REG 0x12
#define MPR121_ELE7_FILTERED_MSB_REG 0x13
#define MPR121_ELE8_FILTERED_LSB_REG 0x14
#define MPR121_ELE8_FILTERED_MSB_REG 0x15
#define MPR121_ELE9_FILTERED_LSB_REG 0x16
#define MPR121_ELE9_FILTERED_MSB_REG 0x17
#define MPR121_ELE10_FILTERED_LSB_REG 0x18
#define MPR121_ELE10_FILTERED_MSB_REG 0x19
#define MPR121_ELE11_FILTERED_LSB_REG 0x1A
#define MPR121_ELE11_FILTERED_MSB_REG 0x1B
#define MPR121_ELEPROX_FILTERED_LSB_REG 0x1C
#define MPR121_ELEPROX_FILTERED_MSB_REG 0x1D

#define MPR121_ELE0_BASELINE_REG 0x1E
#define MPR121_ELE1_BASELINE_REG 0x1F
#define MPR121_ELE2_BASELINE_REG 0x20
#define MPR121_ELE3_BASELINE_REG 0x21
#define MPR121_ELE4_BASELINE_REG 0x22
#define MPR121_ELE5_BASELINE_REG 0x23
#define MPR121_ELE6_BASELINE_REG 0x24
#define MPR121_ELE7_BASELINE_REG 0x25
#define MPR121_ELE8_BASELINE_REG 0x26
#define MPR121_ELE9_BASELINE_REG 0x27
#define MPR121_ELE10_BASELINE_REG 0x28
#define MPR121_ELE11_BASELINE_REG 0x29
#define MPR121_ELEPROX_BASELINE_REG 0x2A


#define MPR121_ELE0_TOUCH_THRESHOLD_REG 0x41

#define MPR121_ELEPROX_TOUCH_THRESHOLD_REG 0x59
#define MPR121_ELEPROX_RELEASE_THRESHOLD_REG 0x5A

#define MPR121_ELE0_CURRENT_REG 0x5F
#define MPR121_ELE1_CURRENT_REG 0x60
#define MPR121_ELE2_CURRENT_REG 0x61
#define MPR121_ELE3_CURRENT_REG 0x62
#define MPR121_ELE4_CURRENT_REG 0x63
#define MPR121_ELE5_CURRENT_REG 0x64
#define MPR121_ELE6_CURRENT_REG 0x65
#define MPR121_ELE7_CURRENT_REG 0x66
#define MPR121_ELE8_CURRENT_REG 0x67
#define MPR121_ELE9_CURRENT_REG 0x68
#define MPR121_ELE10_CURRENT_REG 0x69
#define MPR121_ELE11_CURRENT_REG 0x6A
#define MPR121_ELEPROX_CURRENT_REG 0x6B

#define MPR121_ELE0_ELE1_CHARGE_TIME_REG 0x6C
#define MPR121_ELE2_ELE3_CHARGE_TIME_REG 0x6D
#define MPR121_ELE4_ELE5_CHARGE_TIME_REG 0x6E
#define MPR121_ELE6_ELE7_CHARGE_TIME_REG 0x6F
#define MPR121_ELE8_ELE9_CHARGE_TIME_REG 0x70
#define MPR121_ELE10_ELE11_CHARGE_TIME_REG 0x71
#define MPR121_ELEPROX_CHARGE_TIME_REG 0x72

#define MPR121_AUTO_CONF_CONTROL_0_REG 0x7B
#define MPR121_AUTO_CONF_CONTROL_1_REG 0x7C
#define MPR121_AUTO_UPSIDE_LIMIT_REG 0x7D
#define MPR121_AUTO_LOWSIDE_LIMIT_REG 0x7E
#define MPR121_AUTO_TARGET_LIMIT_REG 0x7F

#define MPR121_SOFT_RESET_REG 0x80
#define MPR121_TOUCH_RELEASE_DEBOUNCE_REG 0x5B

typedef enum {
    MPR121_PROXIMITY_DISABLED = 0x00,
    MPR121_PROXIMITY_ELE0_TO_ELE1_ENABLED = 0x01,
    MPR121_PROXIMITY_ELE0_TO_ELE3_ENABLED = 0x02,
    MPR121_PROXIMITY_ELE0_TO_ELE11_ENABLED = 0x03
} mpr121_enabled_proximity_configuration_t;

typedef enum {
    MPR121_ELE_DETECTION_DISABLED = 0x00,
    MPR121_ELE0_ENABLED = 0x01,
    MPR121_ELE0_TO_ELE1_ENABLED = 0x02,
    MPR121_ELE0_TO_ELE2_ENABLED = 0x03,
    MPR121_ELE0_TO_ELE3_ENABLED = 0x04,
    MPR121_ELE0_TO_ELE4_ENABLED = 0x05,
    MPR121_ELE0_TO_ELE5_ENABLED = 0x06,
    MPR121_ELE0_TO_ELE6_ENABLED = 0x07,
    MPR121_ELE0_TO_ELE7_ENABLED = 0x08,
    MPR121_ELE0_TO_ELE8_ENABLED = 0x09,
    MPR121_ELE0_TO_ELE9_ENABLED = 0x0A,
    MPR121_ELE0_TO_ELE10_ENABLE = 0x0B,
    MPR121_ELE0_TO_ELE11_ENABLED = 0x0C
} mpr121_enabled_electrodes_configuration_t;

typedef enum {
    MPR121_LEVEL_ONE_FILTER_SAMPLES_6 = 0x00,
    MPR121_LEVEL_ONE_FILTER_SAMPLES_10 = 0x01,
    MPR121_LEVEL_ONE_FILTER_SAMPLES_18 = 0x02,
    MPR121_LEVEL_ONE_FILTER_SAMPLES_34 = 0x03
} mpr121_level_one_filter_samples_t;

typedef enum {
    MPR121_BASELINE_ENABLED_WITH_CURRENT_VALUE = 0x00,
    MPR121_BASELINE_DISABLED = 0x01,
    MPR121_BASELINE_ENABLED_WITH_5_HIGH_BITS = 0x02,
    MPR121_BASELINE_ENABLED_WITH_10_HIGH_BITS = 0x03
} mpr121_calibration_lock_t;

//In micro seconds
typedef enum {
    MPR121_DISCHARGE_DISABLED = 0x00
    MPR121_DISCHARGE_TIME_0_5 = 0x01,
    MPR121_DISCHARGE_TIME_1 = 0x02,
    MPR121_DISCHARGE_TIME_2 = 0x03,
    MPR121_DISCHARGE_TIME_4 = 0x04,
    MPR121_DISCHARGE_TIME_8 = 0x05,
    MPR121_DISCHARGE_TIME_16 = 0x06,
    MPR121_DISCHARGE_TIME_32 = 0x07
} mpr121_discharge_time_t;

typedef enum {
    MPR121_LEVEL_TWO_FILTER_SAMPLES_4 = 0x00,
    MPR121_LEVEL_TWO_FILTER_SAMPLES_6 = 0x01,
    MPR121_LEVEL_TWO_FILTER_SAMPLES_10 = 0x02,
    MPR121_LEVEL_TWO_FILTER_SAMPLES_18 = 0x03
} mpr121_level_two_filter_samples_t;

// In Milliseconds
typedef enum {
    MPR121_SAMPLE_INTERVAL_1 = 0x00,
    MPR121_SAMPLE_INTERVAL_2 = 0x01,
    MPR121_SAMPLE_INTERVAL_4 = 0x02,
    MPR121_SAMPLE_INTERVAL_8 = 0x03,
    MPR121_SAMPLE_INTERVAL_16 = 0x04,
    MPR121_SAMPLE_INTERVAL_32 = 0x05,
    MPR121_SAMPLE_INTERVAL_64 = 0x06,
    MPR121_SAMPLE_INTERVAL_128 = 0x07
} mpr121_sample_interval_t;

//In micro ampere. Maximum 63 micro ampere
typedef uint8_t mpr121_discharge_current_t;

typedef enum {
    MPR121_CONFIG_AUTO,
    MPR121_CONFIG_GLOBAL,
    MPR121_CONFIG_MANUAL
} mpr121_configuration_type_t;

typedef struct {
    mpr121_discharge_current_t discharge_current;
    mpr121_discharge_time_t discharge_time;
    uint8_t touch_threshold;
    uint8_t release_threshold;
} mpr121_manual_electrode_configuration_t;

typedef struct {
    mpr121_manual_electrode_configuration_t *electrode_configurations;
    int size;
} mpr121_manual_configuration_t;

typedef enum {
    MPR121_AUTO_NO_RETRY = 0x00,
    MPR121_AUTO_2_RETRY = 0x01,
    MPR121_AUTO_4_RETRY = 0x02,
    MPR121_AUTO_8_RETRY = 0x03
} mpr121_auto_retry_t;

typedef enum {
    MPR121_AUTO_CONFIGURATION_ENABLED = 0x00,
    MPR121_AUTO_CONFIGURATION_DISABLED = 0x01
} mpr121_auto_configuration_enabled_t;

typedef enum {
    MPR121_AUTO_RECONFIGURATION_ENABLED = 0x00,
    MPR121_AUTO_RECONFIGURATION_DISABLED = 0x01
} mpr121_auto_reconfiguration_enabled_t;

typedef enum {
    MPR121_AUTO_SEARCH_CHARGE_TIME_CDT_CDC = 0x00,
    MPR121_AUTO_SEARCH_CHARGE_TIME_SKIP_CDT = 0x01
} mpr121_auto_skip_charge_time_search;

typedef enum {
    MPR121_AUTO_CONFIGURATION_INTERRUPT_DISABLED = 0x00,
    MPR121_AUTO_CONFIGURATION_INTERRUPT_ENABLED = 0x01
} mpr121_auto_configuration_interrupt_t;

typedef enum {
    MPR121_AUTO_RECONFIGURATION_INTERRUPT_DISABLED = 0x00,
    MPR121_AUTO_RECONFIGURATION_INTERRUPT_ENABLED = 0x01
} mpr121_auto_reconfiguration_interrupt_t;

typedef enum {
    MPR121_AUTO_OUT_OF_RANGE_INTERRUPT_DISABLED = 0x00
    MPR121_AUTO_OUT_OF_RANGE_INTERRUPT_ENABLED = 0x01,
} mpr121_auto_out_of_range_interrupt_t;


typedef union {
    struct {
        mpr121_level_one_filter_samples_t level_one_filter_samples;
        mpr121_discharge_current_t discharge_current;
        mpr121_discharge_time_t discharge_time;
        mpr121_level_two_filter_samples_t level_two_filter_samples;
        mpr121_sample_interval_t sample_interval;

        mpr121_manual_configuration_t manual_configuration;
    } global;

    struct {
        mpr121_level_one_filter_samples_t level_one_filter_samples;
        mpr121_auto_configuration_enabled_t auto_configuration_enabled;
        mpr121_auto_reconfiguration_enabled_t auto_reconfiguration_enabled;
        mpr121_auto_skip_charge_time_search skip_charge_time_search;
        mpr121_auto_configuration_interrupt_t auto_configuration_interrupt_enabled;
        mpr121_auto_reconfiguration_interrupt_t auto_reconfiguration_interrupt_enabled;
        mpr121_auto_reconfiguration_enabled_t auto_reconfiguration_enabled;
        mpr121_auto_out_of_range_interrupt_t out_of_range_interrupt;

        mpr121_auto_retry_t retry;
        uint8_t up_side_limit;
        uint8_t low_side_limit;
        uint8_t target_level;
        uint8_t release_threshold;
        uint8_t touch_threshold;
    } automatic;
} mpr121_configuration_t;

typedef enum {
    MPR121_DEVICE_TYPE_BUTTON,
    MPR121_DEVICE_TYPE_FILTERED_DATA,
    MPR121_DEVICE_TYPE_PROXIMITY
} mpr121_detected_as_device_type_t;

typedef struct {
    /* I2C details */
    i2c_t i2c_dev;                      /**< I2C device which is used */
    uint8_t i2c_addr;                   /**< I2C address */

    mpr121_enabled_electrodes_configuration_t enabled_electrodes_configuration;
    mpr121_enabled_proximity_configuration_t enabled_proximity_configuration;
    mpr121_calibration_lock_t calibration_lock;

    mpr121_detected_as_device_type_t detected_as_device;
    mpr121_configuration_type_t configuration_type;
    mpr121_configuration_t configuration;

    // 0 to 7
    uint8_t touch_debounce;
    uint8_t release_debounce;
} mpr121_params_t;

typedef struct {
    mpr121_params_t params;
} mpr121_t;

void mpr121_init_electrode_conf(mpr121_t *dev)
{
    mpr121_configuration_t *config = dev->params.configuration;

    i2c_acquire(dev->params.i2c_dev);
    uint8_t data =
            (dev->params.calibration_lock << 6) |
            (dev->params.enabled_proximity_configuration << 4) |
            (dev->params.enabled_electrodes_configuration);

    i2c_write_reg(dev->params.i2c_dev, dev->params.i2c_addr, MPR121_ELECTRODE_CONF_REG, data, NULL);
    i2c_release(dev->params.i2c_dev);
}

void mpr121_init_debounce_conf(mpr121_t *dev)
{
    mpr121_configuration_t *config = dev->params.configuration;

    i2c_acquire(dev->params.i2c_dev);
    uint8_t data = (dev->params.touch_debounce << 4) | dev->params.release_debounce;
    i2c_write_reg(dev->params.i2c_dev, dev->params.i2c_addr, MPR121_TOUCH_RELEASE_DEBOUNCE_REG, data, NULL);
    i2c_release(dev->params.i2c_dev);
}

void mpr121_set_threshold_conf(){

}

void mpr121_auto_init_threshold_conf(mpr121_t *dev){
    mpr121_configuration_t *config = dev->params.configuration;
    mpr121_enabled_electrodes_configuration_t enabled_electrodes = dev->params.enabled_electrodes_configuration;

    i2c_acquire(dev->params.i2c_dev);
    for(uint8_t i = 0; i <= enabled_electrodes*2; i++){
        uint8_t touch_reg = MPR121_ELE0_TOUCH_THRESHOLD_REG + i;
        uint8_t release_reg = MPR121_ELE0_TOUCH_THRESHOLD_REG + i + 1;

        i2c_write_reg(dev->params.i2c_dev, dev->params.i2c_addr, touch_reg, config->automatic.touch_threshold, NULL);
        i2c_write_reg(dev->params.i2c_dev, dev->params.i2c_addr, release_reg, config->automatic.release_threshold, NULL);
    }

    if(dev->params.enabled_proximity_configuration !== MPR121_PROXIMITY_DISABLED){
        
    }

    i2c_release(dev->params.i2c_dev);
}

void mpr121_init_thresholds_conf(mpr121_t *dev)
{
    mpr121_configuration_t *config = dev->params.configuration;

    i2c_acquire(dev->params.i2c_dev);

    mpr121_enabled_electrodes_configuration_t enabled_electrodes = dev->params.enabled_electrodes_configuration;

    if(dev->params.configuration_type == MPR121_CONFIG_AUTO){
        mpr121_auto_init_threshold_conf();
    }
    if(dev->params.configuration_type == MPR121_CONFIG_GLOBAL){
        mpr121_global_init_threshold_conf();
    }
    if(dev->params.configuration_type == MPR121_CONFIG_MANUAL){
        mpr121_manual_init_threshold_conf();
    }
}

void mpr121_init_auto(mpr121_t *dev)
{
    mpr121_configuration_t *config = dev->params.configuration;

    i2c_acquire(dev->params.i2c_dev);
    uint8_t data =
            (config->automatic.level_one_filter_samples << 6) |
            (config->automatic.retry << 4) |
            (dev->params.calibration_lock << 2) |
            (config->automatic.auto_reconfiguration_enabled << 1) |
            (config->automatic.auto_configuration_enabled);
    i2c_write_reg(dev->params.i2c_dev, dev->params.i2c_addr, MPR121_AUTO_CONF_CONTROL_0_REG, data, NULL);

    data =
            (config->automatic.skip_charge_time_search << 7) |
            (config->automatic.out_of_range_interrupt << 2) |
            (config->automatic.auto_reconfiguration_interrupt_enabled << 1) |
            (config->automatic.auto_configuration_interrupt_enabled);
    i2c_write_reg(dev->params.i2c_dev, dev->params.i2c_addr, MPR121_AUTO_CONF_CONTROL_1_REG, data, NULL);
    i2c_write_reg(dev->params.i2c_dev, dev->params.i2c_addr, MPR121_AUTO_UPSIDE_LIMIT_REG,
                  config->automatic.up_side_limit, NULL);
    i2c_write_reg(dev->params.i2c_dev, dev->params.i2c_addr, MPR121_AUTO_LOWSIDE_LIMIT_REG,
                  config->automatic.low_side_limit, NULL);
    i2c_write_reg(dev->params.i2c_dev, dev->params.i2c_addr, MPR121_AUTO_TARGET_LIMIT_REG,
                  config->automatic.target_level, NULL);

    uint8_t oor_status =
            i2c_read_reg(dev->params.i2c_dev, dev->params.i2c_addr, MPR121_ELE0_TO_ELE7_OOR_STATUS_REG, "", NULL);

    i2c_release(dev->params.i2c_dev);
}

void mpr121_init_global(mpr121_t *dev) {

}

void mpr121_init_manual(mpr121_t *dev) {

}

void mpr121_init(mpr121_t *dev, const mpr121_params_t *params) {
    dev->params = *params;
    mpr121_init_electrode_conf(dev);
    switch (dev->params.configuration_type) {
        case MPR121_CONFIG_AUTO:
            mpr121_init_auto(dev);
            break;
        case MPR121_CONFIG_GLOBAL:
            mpr121_init_global(dev);
            break;
        case MPR121_CONFIG_MANUAL:
            mpr121_init_manual(dev);
            break;
    }
}

int main(void) {
    puts("Hello World!");

    printf("You are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    return 0;
}
