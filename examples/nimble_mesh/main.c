/*
 * Copyright (C) 2018 Freie Universität Berlin
 *               2018 Codecoup
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
 * @brief       BLE peripheral example using NimBLE
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Andrzej Kaczmarek <andrzej.kaczmarek@codecoup.pl>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include <assert.h>

/* BLE */
#include "nimble/ble.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "mesh/mesh.h"
#include "mesh/glue.h"
#include "mesh/cfg_srv.h"
#include "mesh/access.h"
#include "mesh/health_srv.h"

#define BT_DBG_ENABLED (MYNEWT_VAL(BLE_MESH_DEBUG))

/* Company ID */
#define CID_VENDOR 0x05C3
#define STANDARD_TEST_ID 0x00
#define TEST_ID 0x01
static int recent_test_id = STANDARD_TEST_ID;

#define FAULT_ARR_SIZE 2

static bool has_reg_fault = true;

static struct bt_mesh_cfg_srv cfg_srv = {
        .relay = BT_MESH_RELAY_DISABLED,
        .beacon = BT_MESH_BEACON_ENABLED,
#if MYNEWT_VAL(BLE_MESH_FRIEND)
        .frnd = BT_MESH_FRIEND_ENABLED,
#else
        .gatt_proxy = BT_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
#if MYNEWT_VAL(BLE_MESH_GATT_PROXY)
        .gatt_proxy = BT_MESH_GATT_PROXY_ENABLED,
#else
        .gatt_proxy = BT_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
        .default_ttl = 7,

        /* 3 transmissions with 20ms interval */
        .net_transmit = BT_MESH_TRANSMIT(2, 20),
        .relay_retransmit = BT_MESH_TRANSMIT(2, 20),
};

static int
fault_get_cur(struct bt_mesh_model *model,
              uint8_t *test_id,
              uint16_t *company_id,
              uint8_t *faults,
              uint8_t *fault_count)
{
    uint8_t reg_faults[FAULT_ARR_SIZE] = { [0 ... FAULT_ARR_SIZE-1] = 0xff };

    printf("fault_get_cur() has_reg_fault %u\n", has_reg_fault);

    *test_id = recent_test_id;
    *company_id = CID_VENDOR;

    *fault_count = min(*fault_count, sizeof(reg_faults));
    memcpy(faults, reg_faults, *fault_count);

    return 0;
}

static int
fault_get_reg(struct bt_mesh_model *model,
              uint16_t company_id,
              uint8_t *test_id,
              uint8_t *faults,
              uint8_t *fault_count)
{
    if (company_id != CID_VENDOR) {
        return -BLE_HS_EINVAL;
    }

    printf("fault_get_reg() has_reg_fault %u\n", has_reg_fault);

    *test_id = recent_test_id;

    if (has_reg_fault) {
        uint8_t reg_faults[FAULT_ARR_SIZE] = { [0 ... FAULT_ARR_SIZE-1] = 0xff };

        *fault_count = min(*fault_count, sizeof(reg_faults));
        memcpy(faults, reg_faults, *fault_count);
    } else {
        *fault_count = 0;
    }

    return 0;
}

static int
fault_clear(struct bt_mesh_model *model, uint16_t company_id)
{
    if (company_id != CID_VENDOR) {
        return -BLE_HS_EINVAL;
    }

    has_reg_fault = false;

    return 0;
}

static int
fault_test(struct bt_mesh_model *model, uint8_t test_id, uint16_t company_id)
{
    if (company_id != CID_VENDOR) {
        return -BLE_HS_EINVAL;
    }

    if (test_id != STANDARD_TEST_ID && test_id != TEST_ID) {
        return -BLE_HS_EINVAL;
    }

    recent_test_id = test_id;
    has_reg_fault = true;
    bt_mesh_fault_update(bt_mesh_model_elem(model));

    return 0;
}

static const struct bt_mesh_health_srv_cb health_srv_cb = {
        .fault_get_cur = &fault_get_cur,
        .fault_get_reg = &fault_get_reg,
        .fault_clear = &fault_clear,
        .fault_test = &fault_test,
};

static struct bt_mesh_health_srv health_srv = {
        .cb = &health_srv_cb,
};

static struct bt_mesh_model_pub health_pub;

static void
health_pub_init(void)
{
    health_pub.msg  = BT_MESH_HEALTH_FAULT_MSG(0);
}

static struct bt_mesh_model_pub gen_level_pub;
static struct bt_mesh_model_pub gen_onoff_pub;

static uint8_t gen_on_off_state;
static int16_t gen_level_state;

static void gen_onoff_status(struct bt_mesh_model *model,
                             struct bt_mesh_msg_ctx *ctx)
{
    struct os_mbuf *msg = NET_BUF_SIMPLE(3);
    uint8_t *status;

    printf("#mesh-onoff STATUS\n");

    bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_2(0x82, 0x04));
    status = net_buf_simple_add(msg, 1);
    *status = gen_on_off_state;

    if (bt_mesh_model_send(model, ctx, msg, NULL, NULL)) {
        printf("#mesh-onoff STATUS: send status failed\n");
    }

    os_mbuf_free_chain(msg);
}

static void gen_onoff_get(struct bt_mesh_model *model,
                          struct bt_mesh_msg_ctx *ctx,
                          struct os_mbuf *buf)
{
    printf("#mesh-onoff GET\n");

    gen_onoff_status(model, ctx);
}

static void gen_onoff_set(struct bt_mesh_model *model,
                          struct bt_mesh_msg_ctx *ctx,
                          struct os_mbuf *buf)
{
    printf("#mesh-onoff SET\n");

    gen_on_off_state = buf->om_data[0];

    gen_onoff_status(model, ctx);
}

static void gen_onoff_set_unack(struct bt_mesh_model *model,
                                struct bt_mesh_msg_ctx *ctx,
                                struct os_mbuf *buf)
{
    printf("#mesh-onoff SET-UNACK\n");

    gen_on_off_state = buf->om_data[0];

}

static const struct bt_mesh_model_op gen_onoff_op[] = {
        { BT_MESH_MODEL_OP_2(0x82, 0x01), 0, gen_onoff_get },
        { BT_MESH_MODEL_OP_2(0x82, 0x02), 2, gen_onoff_set },
        { BT_MESH_MODEL_OP_2(0x82, 0x03), 2, gen_onoff_set_unack },
        BT_MESH_MODEL_OP_END,
};

static void gen_level_status(struct bt_mesh_model *model,
                             struct bt_mesh_msg_ctx *ctx)
{
    struct os_mbuf *msg = NET_BUF_SIMPLE(4);

    printf("#mesh-level STATUS\n");

    bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_2(0x82, 0x08));
    net_buf_simple_add_le16(msg, gen_level_state);

    if (bt_mesh_model_send(model, ctx, msg, NULL, NULL)) {
        printf("#mesh-level STATUS: send status failed\n");
    }

    os_mbuf_free_chain(msg);
}

static void gen_level_get(struct bt_mesh_model *model,
                          struct bt_mesh_msg_ctx *ctx,
                          struct os_mbuf *buf)
{
    printf("#mesh-level GET\n");

    gen_level_status(model, ctx);
}

static void gen_level_set(struct bt_mesh_model *model,
                          struct bt_mesh_msg_ctx *ctx,
                          struct os_mbuf *buf)
{
    int16_t level;

    level = (int16_t) net_buf_simple_pull_le16(buf);
    printf("#mesh-level SET: level=%d\n", level);

    gen_level_status(model, ctx);

    gen_level_state = level;
    printf("#mesh-level: level=%d\n", gen_level_state);
}

static void gen_level_set_unack(struct bt_mesh_model *model,
                                struct bt_mesh_msg_ctx *ctx,
                                struct os_mbuf *buf)
{
    int16_t level;

    level = (int16_t) net_buf_simple_pull_le16(buf);
    printf("#mesh-level SET-UNACK: level=%d\n", level);

    gen_level_state = level;
    printf("#mesh-level: level=%d\n", gen_level_state);
}

static void gen_delta_set(struct bt_mesh_model *model,
                          struct bt_mesh_msg_ctx *ctx,
                          struct os_mbuf *buf)
{
    int16_t delta_level;

    delta_level = (int16_t) net_buf_simple_pull_le16(buf);
    printf("#mesh-level DELTA-SET: delta_level=%d\n", delta_level);

    gen_level_status(model, ctx);

    gen_level_state += delta_level;
    printf("#mesh-level: level=%d\n", gen_level_state);
}

static void gen_delta_set_unack(struct bt_mesh_model *model,
                                struct bt_mesh_msg_ctx *ctx,
                                struct os_mbuf *buf)
{
    int16_t delta_level;

    delta_level = (int16_t) net_buf_simple_pull_le16(buf);
    printf("#mesh-level DELTA-SET: delta_level=%d\n", delta_level);

    gen_level_state += delta_level;
    printf("#mesh-level: level=%d\n", gen_level_state);
}

static void gen_move_set(struct bt_mesh_model *model,
                         struct bt_mesh_msg_ctx *ctx,
                         struct os_mbuf *buf)
{
}

static void gen_move_set_unack(struct bt_mesh_model *model,
                               struct bt_mesh_msg_ctx *ctx,
                               struct os_mbuf *buf)
{
}

static const struct bt_mesh_model_op gen_level_op[] = {
        { BT_MESH_MODEL_OP_2(0x82, 0x05), 0, gen_level_get },
        { BT_MESH_MODEL_OP_2(0x82, 0x06), 3, gen_level_set },
        { BT_MESH_MODEL_OP_2(0x82, 0x07), 3, gen_level_set_unack },
        { BT_MESH_MODEL_OP_2(0x82, 0x09), 5, gen_delta_set },
        { BT_MESH_MODEL_OP_2(0x82, 0x0a), 5, gen_delta_set_unack },
        { BT_MESH_MODEL_OP_2(0x82, 0x0b), 3, gen_move_set },
        { BT_MESH_MODEL_OP_2(0x82, 0x0c), 3, gen_move_set_unack },
        BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model root_models[] = {
        BT_MESH_MODEL_CFG_SRV(&cfg_srv),
        BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
        BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_ONOFF_SRV, gen_onoff_op,
                      &gen_onoff_pub, NULL),
        BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_LEVEL_SRV, gen_level_op,
                      &gen_level_pub, NULL),
};

static struct bt_mesh_model_pub vnd_model_pub;

static void vnd_model_recv(struct bt_mesh_model *model,
                           struct bt_mesh_msg_ctx *ctx,
                           struct os_mbuf *buf)
{
    struct os_mbuf *msg = NET_BUF_SIMPLE(3);

    printf("#vendor-model-recv\n");

    printf("data:%s len:%d\n", bt_hex(buf->om_data, buf->om_len),
                   buf->om_len);

    bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_3(0x01, CID_VENDOR));
    os_mbuf_append(msg, buf->om_data, buf->om_len);

    if (bt_mesh_model_send(model, ctx, msg, NULL, NULL)) {
        printf("#vendor-model-recv: send rsp failed\n");
    }

    os_mbuf_free_chain(msg);
}

static const struct bt_mesh_model_op vnd_model_op[] = {
        { BT_MESH_MODEL_OP_3(0x01, CID_VENDOR), 0, vnd_model_recv },
        BT_MESH_MODEL_OP_END,
};

static struct bt_mesh_model vnd_models[] = {
        BT_MESH_MODEL_VND(CID_VENDOR, BT_MESH_MODEL_ID_GEN_ONOFF_SRV, vnd_model_op,
                          &vnd_model_pub, NULL),
};

static struct bt_mesh_elem elements[] = {
        BT_MESH_ELEM(0, root_models, vnd_models),
};

static const struct bt_mesh_comp comp = {
        .cid = CID_VENDOR,
        .elem = elements,
        .elem_count = ARRAY_SIZE(elements),
};

static int output_number(bt_mesh_output_action_t action, uint32_t number)
{
    printf("OOB Number: %lu\n", number);

    return 0;
}

static void prov_complete(u16_t net_idx, u16_t addr)
{
    printf("Local node provisioned, primary address 0x%04x\n", addr);
}

static const uint8_t dev_uuid[16] = MYNEWT_VAL(BLE_MESH_DEV_UUID);

static const struct bt_mesh_prov prov = {
        .uuid = dev_uuid,
        .output_size = 4,
        .output_actions = BT_MESH_DISPLAY_NUMBER | BT_MESH_BEEP | BT_MESH_VIBRATE | BT_MESH_BLINK,
        .output_number = output_number,
        .complete = prov_complete,
};

static void
blemesh_on_reset(int reason)
{
    BLE_HS_LOG(ERROR, "Resetting state; reason=%d\n", reason);
}

static void
blemesh_on_sync(void)
{
    int err;
    ble_addr_t addr;

    printf("Bluetooth initialized\n");

    /* Use NRPA */
    err = ble_hs_id_gen_rnd(1, &addr);
    assert(err == 0);
    err = ble_hs_id_set_rnd(addr.val);
    assert(err == 0);

    err = bt_mesh_init(addr.type, &prov, &comp);
    if (err) {
        printf("Initializing mesh failed (err %d)\n", err);
        return;
    }

    printf("Mesh initialized\n");

    if (IS_ENABLED(CONFIG_SETTINGS)) {
        settings_load();
    }

    if (bt_mesh_is_provisioned()) {
        puts("Mesh network restored from flash\n");
    }
}

int main(void){
    puts("Nimble Mesh example");

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = blemesh_on_reset;
    ble_hs_cfg.sync_cb = blemesh_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    health_pub_init();

    return 0;
}