/**
 * This implementation must be C99 & POSIX compatible!
 */

//Std header
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "periph/uart.h"
#include "xtimer.h"

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
#include "proto_compiled/ErrorResponse.pb.h"

#include "crypto/ciphers.h"

//Todo: Check, if these bufers are really needed. Maybe use a ring buffer instead.
char server_transfer_buffer[DID_BUFFER_SIZE];
uint32_t server_transfer_buffer_written_size;
char error_message_buffer[ERROR_BUFFER_SIZE];
uint32_t error_message_buffer_written_size = 0;

char gateway_response_buffer[GATEWAY_RESPONSE_BUFFER_SIZE];
uint32_t gateway_response_buffer_size = 0;

//Fixme: Send back encrypted or hashed DID, instead of clear text.
cipher_t cipher;
uint8_t did_key[AES_KEY_SIZE] = {0}, did_cipher_text[AES_BLOCK_SIZE] = {0};

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

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
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

//Fixme: Status is not updated all the time. Add it everywhere.
uint32_t status_access_status_read_request_count = 0;
uint32_t status_did_write_request_count = 0;
uint32_t status_did_read_request_count = 0;
uint32_t status_error_read_request_count = 0;

uint32_t status_error_count = 0;
uint32_t status_access_granted_count = 0;
uint32_t status_access_denied_count = 0;

uint8_t encode_buffer[ENCODE_BUFFER_SIZE];
size_t encode_buffer_length;

uint8_t gateway_sleep_time_repeated = 0;

iotaDoorLock_ErrorResponse error_response;

//Todo: Same as the buffer variables. Check, if ring buffer can be used.
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

//Todo: Refactoring of command names are possible.
typedef enum {
    DID_WRITE_CMD = 0x41,
    DID_WRITE_GATEWAY_RESPONSE_CMD = 0x42,
    ACCESS_STATUS_READ_CMD = 0x43,
    ACCESS_STATUS_READ_GATEWAY_RESPONSE_CMD = 0x44,
    MESSAGE_END_CMD = 0x00
} door_lock_commands_t;

//Encode
//Fixme: Same name structure as decode.
extern int did_response_encode(uint8_t *buffer, size_t buffer_size, iotaDoorLock_DIDResponse *message_ptr);
extern int access_status_encode(uint8_t *buffer, size_t buffer_size, iotaDoorLock_AccessStatus *message_ptr);
extern int error_response_encode(uint8_t *buffer, size_t buffer_size, iotaDoorLock_ErrorResponse *message_ptr);
extern bool encode_error_message_cb(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

//Decode
extern bool decode_did_schema_cb(pb_istream_t *stream, const pb_field_t *field, void **arg);
extern bool decode_did_method_cb(pb_istream_t *stream, const pb_field_t *field, void **arg);
extern bool decode_did_id_cb(pb_istream_t *stream, const pb_field_t *field, void **arg);
//Fixme: Consistency in function name pattern!
extern int did_request_decode(iotaDoorLock_DIDRequest *message_ptr,
                              uint8_t *encoded_msg_ptr, size_t decoded_msg_size);

iotaDoorLock_DIDRequest did_request;
iotaDoorLock_DIDResponse did_response;
iotaDoorLock_ErrorResponse error_response;

bool initialized_server = false;
bool server_is_running = false;

//Todo: Refactor function name pattern for callbacks.
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
    //Todo: All logs are on DEBUG level at the moment. Some can be changed to INFO level.
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
bool received_did_write_request_response = false;
bool did_write_request_successful = false;

//Todo: Refactor handler function. Use struct to store function ptr?
int handle_did_write_request(struct ble_gatt_access_ctxt *ctxt){
    char func_name[] = "handle_did_write_request";
    log_string(DEBUG, func_name, "server_transfer_buffer", server_transfer_buffer);
    status_did_write_request_count =+ 1;

    int rc = 0;
    uint16_t om_len;
    om_len = OS_MBUF_PKTLEN(ctxt->om);

    rc = ble_hs_mbuf_to_flat(ctxt->om, &server_transfer_buffer,
                             sizeof(server_transfer_buffer), &om_len);

    server_transfer_buffer[om_len] = '\0';
    server_transfer_buffer_written_size = om_len;

    log_string(DEBUG, func_name, "server_transfer_buffer", server_transfer_buffer);

    log_message(DEBUG, func_name, "Bluetooth server", "Decoding message");
    //For checking, if the request is correct. Input validation
    bool decode_status = did_request_decode(&did_request, (uint8_t *) server_transfer_buffer, om_len);

    if(decode_status){
        log_message(DEBUG, func_name, "Bluetooth server", "Successfully decoded message.");
        log_message(DEBUG, func_name, "Bluetooth server", "Sending message to Gateway...");

        char command[] = { DID_WRITE_CMD, '\0' };
        log_string(DEBUG, func_name, "command", command);
        uart_write(GATEWAY_UART_PORT, (const uint8_t *) command, 1);
        uart_write(GATEWAY_UART_PORT, (const uint8_t *) server_transfer_buffer, om_len);

        log_message(DEBUG, func_name, "Bluetooth server", "Sent message to Gateway.");

        log_message(DEBUG, func_name, "Bluetooth server", "Waiting for Gateway response...");
        while(!received_did_write_request_response && gateway_sleep_time_repeated < GATEWAY_REPEAT_SLEEP_TIME){
            gateway_sleep_time_repeated += 1;
            xtimer_usleep(GATEWAY_WAIT_SLEEP_TIME);
        }

        if(!received_did_write_request_response){
            log_message(DEBUG, func_name, "Bluetooth server", "No connection to Gateway. Gateway did not respond.");
            did_response.code = iotaDoorLock_DIDResponse_Code_NO_CONNECTION_TO_GATEWAY;
        }else{
            if(did_write_request_successful){
                log_message(DEBUG, func_name, "Bluetooth server", "Successfully sent message.");
                did_response.code = iotaDoorLock_DIDResponse_Code_SUCCESSFUL_SENT;
            }else{
                log_message(DEBUG, func_name, "Bluetooth server", "An error occurred. Check error endpoint for details.");
                did_response.code = iotaDoorLock_DIDResponse_Code_SEND_ERROR;
            }
        }
    }else{
        did_response.code = iotaDoorLock_DIDResponse_Code_SEND_ERROR;
        char msg[] = "Decoding of did_write_request protobuf message failed.";
        strncpy(error_message_buffer, msg, sizeof(msg));
        error_message_buffer_written_size = sizeof(msg);
        error_response.time = xtimer_now_usec();
    }

    return rc;
}

//Todo: Implement DidResponse
int handle_did_read_request(struct ble_gatt_access_ctxt *ctxt){
    log_string(DEBUG, "handle_did_read_request", "server_transfer_buffer", server_transfer_buffer);

    did_response_encode((uint8_t *) response_buffer, sizeof(response_buffer), &did_response);

    return os_mbuf_append(ctxt->om, &response_buffer,
                          strlen(response_buffer));
}

int handle_error_message_read_request(struct ble_gatt_access_ctxt *ctxt){
    int message_size = error_response_encode(encode_buffer, sizeof(encode_buffer), &error_response);

    strncpy(response_buffer, (char *) encode_buffer, RESPONSE_BUFFER_SIZE);
    log_string(DEBUG, "handle_error_message_read_request", "response_buffer", response_buffer);

    return os_mbuf_append(ctxt->om, &response_buffer, message_size);
}

iotaDoorLock_AccessStatus access_status;
bool got_access_status_response = false;

int handle_access_status_read_request(struct ble_gatt_access_ctxt *ctxt){
    char func_name[] = "handle_access_status_read_request";

    char command[] = { ACCESS_STATUS_READ_CMD, '\0' };
    log_string(DEBUG, func_name, "command", command);
    uart_write(GATEWAY_UART_PORT, (const uint8_t *) command, 1);
    log_message(DEBUG, func_name, "Bluetooth server", "Sent message to Gateway.");

    log_message(DEBUG, func_name, "Bluetooth server", "Waiting for Gateway response...");
    //Todo: Refactoring of blocking wait blocks. Use threads instead for non blocking behavior.
    while(!got_access_status_response && gateway_sleep_time_repeated < GATEWAY_REPEAT_SLEEP_TIME){
        gateway_sleep_time_repeated += 1;
        xtimer_usleep(GATEWAY_WAIT_SLEEP_TIME);
    }

    if(gateway_sleep_time_repeated == GATEWAY_REPEAT_SLEEP_TIME){
        log_message(DEBUG, func_name, "Bluetooth server", "No connection to Gateway. Gateway did not respond.");
        access_status.status_code = iotaDoorLock_AccessStatus_Code_STATUS_NO_CONNECTION_TO_GATEWAY;

        int encoded_size = access_status_encode(encode_buffer, ENCODE_BUFFER_SIZE, &access_status);
        strncpy(response_buffer, (char *) encode_buffer, encoded_size);
    }else{
        strncpy(response_buffer, server_transfer_buffer, strlen(server_transfer_buffer));
    }

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

//Todo: Change name schema and move definition to different file.
pb_callback_t pb_did_method_cb;
pb_callback_t pb_did_schema_cb;
pb_callback_t pb_did_id_cb;
pb_callback_t pb_error_response_message_cb;

bool is_in_command_mode = true;
bool is_write_did_gateway_response_cmd = false;
bool is_read_access_status_gateway_response_cmd = false;

//Todo: Refactor with threads for non-blocking behavior.
void gateway_uart_callback(void *arg, uint8_t data){
    (void) arg;

    char func_name[] = "gateway_callback";

    log_hex(DEBUG, func_name, "data", data);
    log_bool(DEBUG, func_name, "is_in_command_mode", is_in_command_mode);
    log_bool(DEBUG, func_name, "is_write_did_gateway_response_cmd", is_write_did_gateway_response_cmd);
    log_bool(DEBUG, func_name, "is_read_access_status_gateway_response_cmd", is_read_access_status_gateway_response_cmd);

    if(is_in_command_mode){
        switch(data){
            case DID_WRITE_GATEWAY_RESPONSE_CMD:
                is_write_did_gateway_response_cmd = true;
                is_in_command_mode = false;
                break;
            case ACCESS_STATUS_READ_GATEWAY_RESPONSE_CMD:
                is_read_access_status_gateway_response_cmd = true;
                is_in_command_mode = false;
                break;
            default:
                break;
        }
    }else{
        if(is_write_did_gateway_response_cmd){
            if(data == 0x00){
                did_write_request_successful = false;
            }else{
                did_write_request_successful = true;
            }

            is_write_did_gateway_response_cmd = false;
            is_in_command_mode = true;
        } else if(is_read_access_status_gateway_response_cmd){
            if(data != MESSAGE_END_CMD){
                gateway_response_buffer[gateway_response_buffer_size] = data;
                gateway_response_buffer_size += 1;
            }else{

            }
        }
    }
}

void server_init(void) {
    char func_name[] = "server_init";
    log_message(DEBUG, func_name, "Bluetooth server", "Initialize server...");

    uart_poweron(GATEWAY_UART_PORT);
    uart_rx_cb_t gateway_cb = &gateway_uart_callback;
    uart_init(GATEWAY_UART_PORT, GATEWAY_UART_BAUDRATE, gateway_cb, NULL);

    if(!initialized_server){
        int rc = 0;

        pb_did_id_cb.funcs.decode = &decode_did_id_cb;
        pb_did_method_cb.funcs.decode = &decode_did_method_cb;
        pb_did_schema_cb.funcs.decode = &decode_did_schema_cb;
        pb_error_response_message_cb.funcs.encode = &encode_error_message_cb;

        did_request.schema = pb_did_schema_cb;
        did_request.method = pb_did_method_cb;
        did_request.id = pb_did_id_cb;

        error_response.message = pb_error_response_message_cb;

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