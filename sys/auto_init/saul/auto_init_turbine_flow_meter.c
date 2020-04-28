/*
 * Copyright (C) 2020 Philipp-Alexander Blum
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/*
 * @ingroup     sys_auto_init_saul
 * @{
 *
 * @file
 * @brief       Auto initialization for turbine flow meters
 *
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * @}
 */

#ifdef MODULE_TURBINE_FLOW_METER

#include "assert.h"
#include "log.h"
#include "saul_reg.h"
#include "turbine_flow_meter_params.h"
#include "turbine_flow_meter.h"

/**
 * @brief   Allocate memory for the device descriptors
 */
static turbine_flow_meter_t turbine_flow_meter_devs[TURBINE_FLOW_METER_NUM];

/**
 * @brief   Memory for the SAUL registry entries
 */
static saul_reg_t saul_entries[TURBINE_FLOW_METER_NUM*2];

/**
 * @brief   Define the number of saul info
 */
#define TURBINE_FLOW_METER_INFO_NUM ARRAY_SIZE(turbine_flow_meter_saul_reg_info)

/**
 * @name    Import SAUL endpoints
 * @{
 */
extern const saul_driver_t turbine_flow_meter_volume_driver;
extern const saul_driver_t turbine_flow_meter_flow_driver;
/** @} */

void auto_init_turbine_flow_meter(void)
{
    unsigned int se_ix = 0;
    assert(TURBINE_FLOW_METER_NUM == TURBINE_FLOW_METER_INFO_NUM);
    for (unsigned i = 0; i < TURBINE_FLOW_METER_NUM; i ++) {
        LOG_DEBUG("[auto_init_saul] initializing turbine flow meter #%u\n", i);

        int res = turbine_flow_meter_init(&turbine_flow_meter_devs[i], &turbine_flow_meter_params[i]);
        if (res != 0) {
            LOG_ERROR("[auto_init_saul] error initializing turbine_flow_meter #%u\n", i);
        }
        else {
            /* Volume */
            saul_entries[se_ix].dev = &(turbine_flow_meter_devs[i]);
            saul_entries[se_ix].name = turbine_flow_meter_saul_reg_info[i].name;
            saul_entries[se_ix].driver = &turbine_flow_meter_volume_driver;
            saul_reg_add(&(saul_entries[se_ix]));
            se_ix++;

            /* Flow */
            saul_entries[se_ix].dev = &(turbine_flow_meter_devs[i]);
            saul_entries[se_ix].name = turbine_flow_meter_saul_reg_info[i].name;
            saul_entries[se_ix].driver = &turbine_flow_meter_flow_driver;
            saul_reg_add(&(saul_entries[se_ix]));
            se_ix++;
        }
    }
}

#else
typedef int dont_be_pedantic;
#endif /* MODULE_TURBINE_FLOW_METER */
