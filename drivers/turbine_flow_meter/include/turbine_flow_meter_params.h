/*
 * Copyright (C) 2020 Philipp-Alexander Blum
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_turbine_flow_meter
 * @{
 *
 * @file
 * @brief       Generic device driver implementation for turbine flow meters
 *
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * @}
 */

#ifndef TURBINE_FLOW_METER_PARAMS_H
#define TURBINE_FLOW_METER_PARAMS_H

#include "board.h"
#include "turbine_flow_meter.h"
#include "saul_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters for turbine flow meter
 * @{
 */

#ifndef TURBINE_FLOW_METER_PARAM_PIN
#define TURBINE_FLOW_METER_PARAM_PIN (GPIO_PIN(PORT_A, 1))
#endif
#ifndef TURBINE_FLOW_METER_PARAM_K_FACTOR
#define TURBINE_FLOW_METER_PARAM_K_FACTOR 450
#endif
#ifndef TURBINE_FLOW_METER_PARAMS
#define TURBINE_FLOW_METER_PARAMS                           \
    {                                                       \
        .pin        = TURBINE_FLOW_METER_PARAM_PIN,         \
        .k_factor   = TURBINE_FLOW_METER_PARAM_K_FACTOR,    \
    }

#endif
#ifndef TURBINE_FLOW_METER_SAULINFO
#define TURBINE_FLOW_METER_SAULINFO                { .name = "turbine_flow_meter" }
#endif
/**@}*/

/**
 * @brief   Configure turbine flow meters
 */
static const turbine_flow_meter_params_t turbine_flow_meter_params[] =
        {
                TURBINE_FLOW_METER_PARAMS
        };

/**
 * @brief   The number of configured turbine flow meters
 */
#define TURBINE_FLOW_METER_NUM    ARRAY_SIZE(turbine_flow_meter_params)

/**
 * @brief   Allocate and configure entries for the SAUL registry
 * Override it, if you want to have more than one turbine flow meter.
 * Or, if you want to have turbine flow meters with different names in the saul info registry.
 */
static const saul_reg_info_t turbine_flow_meter_saul_reg_info[TURBINE_FLOW_METER_NUM] =
        {
                TURBINE_FLOW_METER_SAULINFO
        };

#ifdef __cplusplus
}
#endif

#endif /* TURBINE_FLOW_METER_PARAMS_H */
