/**
 * This implementation must be C99 & POSIX compatible!
 */

//Std header
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "logging.h"

//POSIX
#include "pthread.h"

//BLE communication
#include "nimble_riot.h"
#include "net/bluetil/ad.h"

#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

//Encoding / Decoding
#include "proto_compiled/AccessStatus.pb.h"
#include "proto_compiled/DIDRequest.pb.h"
#include "proto_compiled/DIDResponse.pb.h"

#include "crypto/ciphers.h"
#include "periph/spi.h"

char server_transfer_buffer[DID_BUFFER_SIZE];
char error_message_buffer[ERROR_BUFFER_SIZE];

cipher_t cipher;
uint8_t did_key[AES_KEY_SIZE] = {0},
    did_cipher_text[AES_BLOCK_SIZE] = {0};

static const ble_uuid128_t gatt_svr_svc_rw_door_lock_uuid
    = BLE_UUID128_INIT( GATT_SERVICE_UUID );

static const ble_uuid128_t gatt_svr_svc_chr_w_did_uuid
    = BLE_UUID128_INIT( GATT_DID_CHARACTERISTIC_UUID );

static const ble_uuid128_t gatt_svr_svc_chr_r_access_status_uuid
    = BLE_UUID128_INIT( GATT_ACCESS_STATUS_CHARACTERISTIC_UUID );

static const ble_uuid128_t gatt_svr_chr_r_error_message_uuid
    = BLE_UUID128_INIT( GATT_ERROR_MESSAGE_CHARACTERISTIC_UUID );

static const char *device_name = DEVICE_NAME;

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
        .characteristics = (struct ble_gatt_chr_def[])
            {
                {
                    /* Characteristic: * Manufacturer name */
                    .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
                    .access_cb = gatt_svr_chr_access_device_info_manufacturer,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {
                    /* Characteristic: Model number string */
                    .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
                    .access_cb = gatt_svr_chr_access_device_info_model,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {
                    0, /* No more characteristics in this service */
                },
            }
    },
    {
        /* Service: Read/Write Door lock access */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = (ble_uuid_t * ) & gatt_svr_svc_rw_door_lock_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[])
            {
                {
                    /* Characteristic: Read/Write DID */
                    .uuid = (ble_uuid_t * ) &
                            gatt_svr_svc_chr_w_did_uuid.u,
                    .access_cb = gatt_svr_chr_access_rw_door_lock,
                    .flags = BLE_GATT_CHR_F_READ |
                             BLE_GATT_CHR_F_WRITE,
                },
                {
                    /* Characteristic: Read access status */
                    .uuid = (ble_uuid_t * ) &
                            gatt_svr_svc_chr_r_access_status_uuid.u,
                    .access_cb = gatt_svr_chr_access_rw_door_lock,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {
                    /* Characteristic: Read Error Message */
                    .uuid = (ble_uuid_t * ) &
                            gatt_svr_chr_r_error_message_uuid.u,
                    .access_cb = gatt_svr_chr_access_rw_door_lock,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {
                    0, /* No more characteristics in this service */
                },
            }
    },
    {
        0, /* No more services */
    },
};

uint32_t status_access_status_read_request_count = 0;
uint32_t status_did_write_request_count = 0;
uint32_t status_did_read_request_count = 0;
uint32_t status_error_read_request_count = 0;

uint32_t status_error_count = 0;
uint32_t status_access_granted_count = 0;
uint32_t status_access_denied_count = 0;

uint8_t encode_buffer[ENCODE_BUFFER_SIZE];
size_t encode_buffer_length;

void clear_encode_buffer(void) {
    memset(encode_buffer, 0, ENCODE_BUFFER_SIZE);
    encode_buffer_length = 0;
}

static char response_buffer[RESPONSE_BUFFER_SIZE];
size_t response_buffer_length;

void clear_response_buffer(void) {
    memset(response_buffer, 0, RESPONSE_BUFFER_SIZE);
    response_buffer_length = 0;
}

typedef enum {
    DID_CMD = 0x1,
    ACCESS_STATUS_CMD = 0x2
} door_lock_commands_t;

//Encode
extern int did_response_encode(uint8_t *buffer, size_t buffer_size, iotaDoorLock_DIDResponse *message_ptr);
extern int access_status_encode(uint8_t *buffer, size_t buffer_size, iotaDoorLock_AccessStatus *message_ptr);

//Decode
extern bool decode_did_schema_cb(pb_istream_t *stream, const pb_field_t *field, void **arg);
extern bool decode_did_method_cb(pb_istream_t *stream, const pb_field_t *field, void **arg);
extern bool decode_did_id_cb(pb_istream_t *stream, const pb_field_t *field, void **arg);
extern int did_request_decode(iotaDoorLock_DIDRequest *message_ptr,
                              uint8_t *encoded_msg_ptr, size_t decoded_msg_size);

iotaDoorLock_DIDRequest did_request;
iotaDoorLock_DIDResponse did_response;

bool initialized_server = false;
bool server_is_running = false;

static int gap_event_cb(struct ble_gap_event *event, void *arg) {
    (void) arg;
    char func_name[] = "gap_event_cb";

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            log_message(DEBUG, func_name, "Bluetooth server", "New device connected to server.");

            if (event->connect.status) {
                start_advertise();
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            log_message(DEBUG, func_name, "Bluetooth server", "Device disconnected from server.");
            start_advertise();
            break;
    }

    return 0;
}

static int gatt_svr_chr_access_device_info_manufacturer(
    uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    char func_name[] = "gatt_svr_chr_access_device_info_manufacturer";
    log_message(DEBUG, func_name, "Bluetooth server", "callback of service 'device info: manufacturer' triggered.");

    (void) conn_handle;
    (void) attr_handle;
    (void) arg;

    snprintf(response_buffer, RESPONSE_BUFFER_SIZE, DEVICE_INFO_MANUFACTURER, RIOT_VERSION);

    log_string(DEBUG, func_name, "response_buffer", response_buffer);

    return os_mbuf_append(ctxt->om, response_buffer, strlen(response_buffer));;
}

static int gatt_svr_chr_access_device_info_model(
    uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    char func_name[] = "gatt_svr_chr_access_device_info_model";
    log_message(DEBUG, func_name, "Bluetooth server", "callback of service 'device info: model' triggered.");

    (void) conn_handle;
    (void) attr_handle;
    (void) arg;

    snprintf(response_buffer, RESPONSE_BUFFER_SIZE, DEVICE_INFO_MODEL, RIOT_MCU);

    log_string(DEBUG, func_name, "response_buffer", response_buffer);

    return os_mbuf_append(ctxt->om, response_buffer, strlen(response_buffer));
}

iotaDoorLock_DIDResponse decode_response;
int handle_did_write_request(struct ble_gatt_access_ctxt *ctxt){
    char func_name[] = "handle_did_write_request";
    log_string(DEBUG, func_name, "server_transfer_buffer", server_transfer_buffer);
    status_did_write_request_count =+ 1;

    int rc = 0;
    uint16_t om_len;
    om_len = OS_MBUF_PKTLEN(ctxt->om);

    /* read sent data */
    rc = ble_hs_mbuf_to_flat(ctxt->om, &server_transfer_buffer,
                             sizeof(server_transfer_buffer), &om_len);
    /* we need to null-terminate the received string */
    server_transfer_buffer[om_len] = '\0';

    log_string(DEBUG, func_name, "server_transfer_buffer", server_transfer_buffer);

    bool decode_status = did_request_decode(&did_request, (uint8_t *) server_transfer_buffer, om_len);

    if(!decode_status){
        did_response.code = iotaDoorLock_DIDResponse_Code_SEND_ERROR;
    }else{
        did_response.code = iotaDoorLock_DIDResponse_Code_SUCCESSFUL_SEND;

        spi_acquire(GATEWAY_SPI_BUS, GATEWAY_SPI_CS_PIN, GATEWAY_SPI_MODE, SPI_CLK_10MHZ);
        spi_transfer_byte (GATEWAY_SPI_BUS, GATEWAY_SPI_CS_PIN, true, DID_CMD);
        spi_transfer_bytes(GATEWAY_SPI_BUS, GATEWAY_SPI_CS_PIN, false, server_transfer_buffer, NULL, om_len);
        spi_release(GATEWAY_SPI_BUS);
    }

    return rc;
}

int handle_did_read_request(struct ble_gatt_access_ctxt *ctxt){
    log_string(DEBUG, "handle_did_read_request", "server_transfer_buffer", server_transfer_buffer);

    did_response_encode((uint8_t *) response_buffer, sizeof(response_buffer), &did_response);

    return os_mbuf_append(ctxt->om, &response_buffer,
                          strlen(response_buffer));
}

int handle_error_message_read_request(struct ble_gatt_access_ctxt *ctxt){
    strncpy(response_buffer, error_message_buffer, RESPONSE_BUFFER_SIZE);
    log_string(DEBUG, "handle_error_message_read_request", "response_buffer", response_buffer);

    return os_mbuf_append(ctxt->om, &response_buffer, strlen(response_buffer));
}

iotaDoorLock_AccessStatus access_status;

int handle_access_status_read_request(struct ble_gatt_access_ctxt *ctxt){
    spi_acquire(GATEWAY_SPI_BUS, GATEWAY_SPI_CS_PIN, GATEWAY_SPI_MODE, SPI_CLK_10MHZ);
    uint8_t command = ACCESS_STATUS_CMD;
    spi_transfer_bytes(GATEWAY_SPI_BUS, GATEWAY_SPI_CS_PIN, false, &command, server_transfer_buffer, 1);
    spi_release(GATEWAY_SPI_BUS);

    strncpy(response_buffer, server_transfer_buffer, strlen(server_transfer_buffer));
    log_string(DEBUG, "handle_access_status_read_request", "response_buffer", response_buffer);

    return os_mbuf_append(ctxt->om, &response_buffer, strlen(response_buffer));
}

static int gatt_svr_chr_access_rw_door_lock(
    uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    char func_name[] = "gatt_svr_chr_access_rw_door_lock";

    log_message(DEBUG, func_name, "Bluetooth server", "callback of service 'rw_door_lock' triggered.");

    (void) conn_handle;
    (void) attr_handle;
    (void) arg;

    int rc = 0;

    ble_uuid_t* write_did_uuid = (ble_uuid_t*) &gatt_svr_svc_chr_w_did_uuid.u;
    ble_uuid_t* readonly_access_status_uuid = (ble_uuid_t*) &gatt_svr_svc_chr_r_access_status_uuid.u;
    ble_uuid_t* readonly_error_message_uuid = (ble_uuid_t*) &gatt_svr_chr_r_error_message_uuid.u;

    if (ble_uuid_cmp(ctxt->chr->uuid, write_did_uuid) == 0) {
        log_message(DEBUG, func_name, "Bluetooth server", "access to 'w_did' (write) characteristics.");

        switch (ctxt->op) {

            case BLE_GATT_ACCESS_OP_READ_CHR:
                rc = handle_did_read_request(ctxt);
                break;

            case BLE_GATT_ACCESS_OP_WRITE_CHR:
                rc = handle_did_write_request(ctxt);
                break;

            case BLE_GATT_ACCESS_OP_READ_DSC:
                log_message(DEBUG, func_name, "Bluetooth server", "read from 'w_did' (write) descriptor.");
                break;

            case BLE_GATT_ACCESS_OP_WRITE_DSC:
                break;

            default:
                log_message(DEBUG, func_name, "Bluetooth server", "Unhandled operation in 'w_did' (write).");
                rc = 1;
                break;
        }

        return rc;
    }
    else if (ble_uuid_cmp(ctxt->chr->uuid, readonly_error_message_uuid) == 0) {
        log_message(DEBUG, func_name, "Bluetooth server", "access to 'r_error_message' (read-only) characteristics.");

        if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
            return handle_error_message_read_request(ctxt);
        }

        return 0;
    }
    else if (ble_uuid_cmp(ctxt->chr->uuid, readonly_access_status_uuid) == 0) {
        log_message(DEBUG, func_name, "Bluetooth server", "access to 'r_access_status' (read-only) characteristics.");

        if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
            return handle_access_status_read_request(ctxt);
        }

        return 0;
    }

    log_message(ERROR, func_name, "Bluetooth server", "Unhandled UUID!");
    return 1;
}


static void start_advertise(void) {
    char func_name[] = "start_advertise";
    log_message(DEBUG, func_name, "Bluetooth server", "Start Bluetooth advertising...");

    struct ble_gap_adv_params advp;
    int rc;

    memset(&advp, 0, sizeof advp);
    advp.conn_mode = BLE_GAP_CONN_MODE_UND;
    advp.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER,
                           &advp, gap_event_cb, NULL);
    assert(rc == 0);
    log_message(DEBUG, func_name, "Bluetooth server", "Bluetooth advertising started.");
    (void) rc;
}

pb_callback_t pb_did_method_cb;
pb_callback_t pb_did_schema_cb;
pb_callback_t pb_did_id_cb;

void server_init(void) {
    char func_name[] = "server_init";
    log_message(DEBUG, func_name, "Bluetooth server", "Initialize server...");

    spi_init(GATEWAY_SPI_BUS);
    spi_init_cs(GATEWAY_SPI_BUS,GATEWAY_SPI_CS_PIN);

    if(!initialized_server){
        int rc = 0;

        pb_did_id_cb.funcs.decode = &decode_did_id_cb;
        pb_did_method_cb.funcs.decode = &decode_did_method_cb;
        pb_did_schema_cb.funcs.decode = &decode_did_schema_cb;

        log_message(DEBUG, func_name, "Bluetooth server", "Verify and add our custom services...");
        rc = ble_gatts_count_cfg(gatt_svr_svcs);
        assert(rc == 0);
        rc = ble_gatts_add_svcs(gatt_svr_svcs);
        assert(rc == 0);
        log_message(DEBUG, func_name, "Bluetooth server", "Verification done.");


        log_message(DEBUG, func_name, "Bluetooth server", "Set the device name...");
        ble_svc_gap_device_name_set(device_name);
        log_message(DEBUG, func_name, "Bluetooth server", "Set the device name done.");


        log_message(DEBUG, func_name, "Bluetooth server", "Reload the GATT server to link our added services...");
        ble_gatts_start();
        log_message(DEBUG, func_name, "Bluetooth server", "Reloaded GATT server.");


        log_message(DEBUG, func_name, "Bluetooth server", "Configure and set the advertising data...");
        uint8_t buf[BLE_HS_ADV_MAX_SZ];
        bluetil_ad_t ad;
        bluetil_ad_init_with_flags(&ad, buf, sizeof(buf), BLUETIL_AD_FLAGS_DEFAULT);
        bluetil_ad_add_name(&ad, device_name);
        ble_gap_adv_set_data(ad.buf, ad.pos);
        log_message(DEBUG, func_name, "Bluetooth server", "Server configured.");

        initialized_server = true;
        log_message(DEBUG, func_name, "Bluetooth server", "Server initialized.");
    }else{
        log_message(DEBUG, func_name, "Bluetooth server", "Server already initialized.");
    }
}

void server_start_listening(void) {
    server_init();
    server_is_running = true;
    /* start to advertise this node */
    start_advertise();
}

void server_stop(void) {
    ble_gap_adv_stop();
    server_is_running = false;

    status_error_count = 0;
    status_access_denied_count = 0;
    status_access_granted_count = 0;

    status_access_status_read_request_count = 0;
    status_did_read_request_count = 0;
    status_did_write_request_count = 0;
    status_error_read_request_count = 0;
}

void server_status(void) {
    //Todo: Rethink it. Maybe return a struct instead. Log instead in shell_cmds
    printf("-----Server status reporting-----\n\n");
    printf("\t-----Control-----\n");
    printf("\tError count: %lu\n", status_error_count);
    printf("\tGranted access: %lu times\n", status_access_granted_count);
    printf("\tGranted denied: %lu times\n", status_access_denied_count);

    printf("\n\t-----Communication------\n");
    printf("\tAccess status read requests: %lu\n", status_access_status_read_request_count);
    printf("\tDID read requests: %lu\n", status_did_read_request_count);
    printf("\tDID write requests: %lu\n", status_did_write_request_count);
    printf("\tError read requests: %lu\n", status_error_read_request_count);
}

bool is_server_running(void) {
    return server_is_running;
}

void *run_server_thread(void *args) {
    log_message(DEBUG, "run_server_thread", "Bluetooth server", "Start listening...");

    (void) args;

    server_start_listening();
    int value = 0;
    pthread_exit(&value);
    return 0;
}