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
 * @brief       SAUL adaption for turbine flow meter
 *
 *
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * @}
 */

#include <string.h>
#include "xtimer.h"

#include "saul.h"
#include "turbine_flow_meter.h"

static int read_volume(const void *dev, phydat_t *res)
{
    res->val[0] = turbine_flow_meter_volume_read((turbine_flow_meter_t *) dev);
    res->unit = UNIT_M3;
    res->scale = -3;
    printf("Volume: %i\n", res->val[0]);
    return 1;
}

static int read_flow(const void *dev, phydat_t *res)
{
    res->val[0] = (uint16_t) turbine_flow_meter_flow_read((turbine_flow_meter_t *) dev);
    res->unit = UNIT_M3_SEC;
    res->scale = -3;
    printf("Flow: %i\n", res->val[0]);
    return 1;
}

static int reset_volume(const void *dev, phydat_t *data)
{
    turbine_flow_meter_volume_reset((turbine_flow_meter_t *) dev, data->val[0]);
    return 1;
}

static int reset_flow(const void *dev, phydat_t *data)
{
    turbine_flow_meter_flow_reset((turbine_flow_meter_t *) dev, data->val[0], xtimer_now_usec64());
    return 1;
}

const saul_driver_t turbine_flow_meter_volume_driver = {
        .read = read_volume,
        .write = reset_volume,
        .type = SAUL_SENSE_VOLUME
};

const saul_driver_t turbine_flow_meter_flow_driver = {
        .read = read_flow,
        .write = reset_flow,
        .type = SAUL_SENSE_FLOW
};
