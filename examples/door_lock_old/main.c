#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nimble_riot.h"
#include "net/bluetil/ad.h"

#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define GATT_DEVICE_INFO_UUID                   0x180A
#define GATT_MANUFACTURER_NAME_UUID             0x2A29
#define GATT_MODEL_NUMBER_UUID                  0x2A28

#define STR_ANSWER_BUFFER_SIZE 100

static const ble_uuid128_t gatt_svr_svc_rw_door_lock_uuid
        = BLE_UUID128_INIT(                                                                                                                                              , 0xa3, 0xc7, 0x14, 0x3f, 0x03, 0x3e, 0xa1, 0xfc,
                           0x48, 0x37, 0xd1, 0xb3, 0x38, 0xce, 0x1b);

static const ble_uuid128_t gatt_svr_svc_chr_w_did_uuid
        = BLE_UUID128_INIT(0x35, 0xa3, 0xc7, 0x14, 0x3e, 0x03, 0x3e, 0xa1, 0xfa,
                0x48, 0x37, 0xd1, 0xb3, 0x38, 0xce, 0x1b);

static const ble_uuid128_t gatt_svr_svc_chr_r_access_status_uuid
        = BLE_UUID128_INIT(0x35, 0xa3, 0xc7, 0x14, 0x3e, 0x03, 0x3e, 0xa1, 0xfb,
                           0x48, 0x37, 0xd1, 0xb3, 0x38, 0xce, 0x1b);

static const ble_uuid128_t gatt_svr_chr_r_error_message_uuid
        = BLE_UUID128_INIT(0x62, 0x17, 0x99, 0x7e, 0x50, 0x27, 0x38, 0xba, 0x3b,
                0x4f, 0x70, 0x30, 0x86, 0x83, 0xf2, 0x35);

static const char *device_name = "IOTA door lock";

static char did_value[64] = "";

static int gatt_svr_chr_access_device_info_manufacturer(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_device_info_model(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_rw_door_lock(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

static void start_advertise(void);

static char str_answer[STR_ANSWER_BUFFER_SIZE];

/* define several bluetooth services for our device */
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    /*
     * access_cb defines a callback for read and write access events on
     * given characteristics
     */
    {
        /* Service: Device Information */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            /* Characteristic: * Manufacturer name */
            .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
            .access_cb = gatt_svr_chr_access_device_info_manufacturer,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            /* Characteristic: Model number string */
            .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
            .access_cb = gatt_svr_chr_access_device_info_model,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            0, /* No more characteristics in this service */
        }, }
    },
    {
        /* Service: Read/Write Door lock access */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = (ble_uuid_t*) &gatt_svr_svc_rw_door_lock_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) { {
            /* Characteristic: Read/Write DID */
            .uuid = (ble_uuid_t*) &gatt_svr_svc_chr_w_did_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_door_lock,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
                /* Characteristic: Read access status */
                .uuid = (ble_uuid_t*) &gatt_svr_svc_chr_r_access_status_uuid.u,
                .access_cb = gatt_svr_chr_access_rw_door_lock,
                .flags = BLE_GATT_CHR_F_READ,
        }, {
            /* Characteristic: Read Error Message */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_r_error_message_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_door_lock,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            0, /* No more characteristics in this service */
        }, }
    },
    {
        0, /* No more services */
    },
};

static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status) {
                start_advertise();
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            start_advertise();
            break;
    }

    return 0;
}

static void start_advertise(void)
{
    struct ble_gap_adv_params advp;
    int rc;

    memset(&advp, 0, sizeof advp);
    advp.conn_mode = BLE_GAP_CONN_MODE_UND;
    advp.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
                           &advp, gap_event_cb, NULL);
    assert(rc == 0);
    (void)rc;
}

static int gatt_svr_chr_access_device_info_manufacturer(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    puts("service 'device info: manufacturer' callback triggered");

    (void) conn_handle;
    (void) attr_handle;
    (void) arg;

    snprintf(str_answer, STR_ANSWER_BUFFER_SIZE,
             "IOTA HID door lock (Version: %s)\n", RIOT_VERSION);
    puts(str_answer);

    int rc = os_mbuf_append(ctxt->om, str_answer, strlen(str_answer));

    puts("");

    return rc;
}

static int gatt_svr_chr_access_device_info_model(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    puts("service 'device info: model' callback triggered");

    (void) conn_handle;
    (void) attr_handle;
    (void) arg;

    snprintf(str_answer, STR_ANSWER_BUFFER_SIZE,
             "IOTA HID door lock %s with %s MCU.", RIOT_BOARD, RIOT_MCU);
    puts(str_answer);

    int rc = os_mbuf_append(ctxt->om, str_answer, strlen(str_answer));

    puts("");

    return rc;
}

static int gatt_svr_chr_access_rw_door_lock(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    puts("service 'rw door_lock' callback triggered");

    (void) conn_handle;
    (void) attr_handle;
    (void) arg;

    int rc = 0;

    ble_uuid_t* write_did_uuid = (ble_uuid_t*) &gatt_svr_svc_chr_w_did_uuid.u;
    ble_uuid_t* readonly_access_status_uuid = (ble_uuid_t*) &gatt_svr_svc_chr_r_access_status_uuid.u;
    ble_uuid_t* readonly_error_message_uuid = (ble_uuid_t*) &gatt_svr_chr_r_error_message_uuid.u;

    if (ble_uuid_cmp(ctxt->chr->uuid, write_did_uuid) == 0) {

        puts("access to characteristic 'w DID (write)'");

        switch (ctxt->op) {

            case BLE_GATT_ACCESS_OP_READ_CHR:
                puts("read from characteristic");
                printf("current value of rm_demo_write_data: '%s'\n",
                       rm_demo_write_data);

                /* send given data to the client */
                rc = os_mbuf_append(ctxt->om, &rm_demo_write_data,
                                    strlen(rm_demo_write_data));

                break;

            case BLE_GATT_ACCESS_OP_WRITE_CHR:
                puts("write to characteristic");

                printf("old value of rm_demo_write_data: '%s'\n",
                       rm_demo_write_data);

                uint16_t om_len;
                om_len = OS_MBUF_PKTLEN(ctxt->om);

                /* read sent data */
                rc = ble_hs_mbuf_to_flat(ctxt->om, &rm_demo_write_data,
                                         sizeof rm_demo_write_data, &om_len);
                /* we need to null-terminate the received string */
                rm_demo_write_data[om_len] = '\0';

                printf("new value of rm_demo_write_data: '%s'\n",
                       rm_demo_write_data);

                break;

            case BLE_GATT_ACCESS_OP_READ_DSC:
                puts("read from descriptor");
                break;

            case BLE_GATT_ACCESS_OP_WRITE_DSC:
                puts("write to descriptor");
                break;

            default:
                puts("unhandled operation!");
                rc = 1;
                break;
        }

        puts("");

        return rc;
    }
    else if (ble_uuid_cmp(ctxt->chr->uuid, readonly_error_message_uuid) == 0) {

        puts("access to characteristic 'r error_message (read-only)'");

        if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
            char random_digit;
            /* get random char between '0' and '9' */
            random_digit = 48 + (rand() % 10);

            snprintf(str_answer, STR_ANSWER_BUFFER_SIZE,
                     "error %c", random_digit);
            puts(str_answer);

            rc = os_mbuf_append(ctxt->om, &str_answer, strlen(str_answer));

            puts("");

            return rc;
        }

        return 0;
    }
    else if (ble_uuid_cmp(ctxt->chr->uuid, readonly_access_status_uuid) == 0) {

        puts("access to characteristic 'r access_status (read-only)'");

        if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
            snprintf(str_answer, STR_ANSWER_BUFFER_SIZE,
                     "access not granted.");
            puts(str_answer);

            rc = os_mbuf_append(ctxt->om, &str_answer, strlen(str_answer));

            puts("");

            return rc;
        }

        return 0;
    }

    puts("unhandled uuid!");
    return 1;
}

int main(void)
{
    puts("IOTA HID door lock");

    int rc = 0;

    /* verify and add our custom services */
    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    assert(rc == 0);
    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    assert(rc == 0);

    /* set the device name */
    ble_svc_gap_device_name_set(device_name);
    /* reload the GATT server to link our added services */
    ble_gatts_start();

    /* configure and set the advertising data */
    uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bluetil_ad_t ad;
    bluetil_ad_init_with_flags(&ad, buf, sizeof(buf), BLUETIL_AD_FLAGS_DEFAULT);
    bluetil_ad_add_name(&ad, device_name);
    ble_gap_adv_set_data(ad.buf, ad.pos);

    /* start to advertise this node */
    start_advertise();

    return 0;
}
