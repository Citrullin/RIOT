//Std header
#include <stdio.h>
#include <string.h>

//Nanopb
#include "pb_common.h"
#include "pb_encode.h"

//Nanopb compiled
#include "proto_compiled/AccessStatus.pb.h"
#include "proto_compiled/DIDResponse.pb.h"
#include "proto_compiled/ErrorResponse.pb.h"
#include "proto_compiled/DIDRequest.pb.h"

#include "config.h"
#include "logging.h"

extern char error_message_buffer[ERROR_BUFFER_SIZE];
extern uint32_t error_message_buffer_written_size;

void log_encode_result(char * func_name, bool encode_status, size_t encode_message_length){
    log_bool(DEBUG, func_name, "encode_status", encode_status);
    log_int(DEBUG, func_name, "encode_message_length", encode_message_length);
}

void log_access_status(char * func_name, iotaDoorLock_AccessStatus * access_status_ptr){
    static char access_status_name[40] = "";
    switch(access_status_ptr->status_code){
        case iotaDoorLock_AccessStatus_Code_STATUS_GRANTED:
            strncpy(access_status_name, "STATUS_GRANTED", sizeof(access_status_name));
            break;
        case iotaDoorLock_AccessStatus_Code_STATUS_DENIED:
            strncpy(access_status_name, "STATUS_DENIED", sizeof(access_status_name));
            break;
        case iotaDoorLock_AccessStatus_Code_STATUS_ERROR:
            strncpy(access_status_name, "STATUS_ERROR", sizeof(access_status_name));
            break;
        case iotaDoorLock_AccessStatus_Code_STATUS_NO_CONNECTION_TO_GATEWAY:
            strncpy(access_status_name, "STATUS_NO_CONNECTION_TO_GATEWAY", sizeof(access_status_name));
            break;
    }
    log_string(DEBUG, func_name, "access_status", access_status_name);
}

void log_did_response(char * func_name, iotaDoorLock_DIDResponse * did_response_ptr) {
    static char did_response_code_name[20] = "";
    switch (did_response_ptr->code) {
        case iotaDoorLock_DIDResponse_Code_SUCCESSFUL_SENT:
            strncpy(did_response_code_name, "SUCCESSFUL_SENT", sizeof(did_response_code_name));
            break;
        case iotaDoorLock_DIDResponse_Code_SEND_ERROR:
            strncpy(did_response_code_name, "SEND_ERROR", sizeof(did_response_code_name));
            break;
        case iotaDoorLock_DIDResponse_Code_NO_CONNECTION_TO_GATEWAY:
            strncpy(did_response_code_name, "NO_CONNECTION_TO_GATEWAY", sizeof(did_response_code_name));
            break;
    }

    log_string(DEBUG, func_name, "did_response", did_response_code_name);
}

extern pb_callback_t pb_did_method_cb;
extern pb_callback_t pb_did_schema_cb;
extern pb_callback_t pb_did_id_cb;

extern pb_byte_t decode_schema_buffer[DECODE_SCHEMA_SIZE];
extern pb_byte_t decode_method_buffer[DECODE_METHOD_SIZE];
extern pb_byte_t decode_id_buffer[DECODE_ID_SIZE];

uint32_t encode_did_id_bytes_written;
uint32_t encode_did_method_bytes_written;
uint32_t encode_did_schema_bytes_written;

bool encode_did_id_cb(pb_ostream_t *stream, const pb_field_t *field, void * const *arg){
    (void) arg;
    (void) field;
    log_string(DEBUG, "encode_did_id_cb", "error_message_buffer", error_message_buffer);


    return pb_encode_string(stream, (pb_byte_t *) decode_id_buffer, encode_did_id_bytes_written);
}

bool encode_did_method_cb(pb_ostream_t *stream, const pb_field_t *field, void * const *arg){
    (void) arg;
    (void) field;
    log_string(DEBUG, "encode_did_method_cb", "error_message_buffer", error_message_buffer);


    return pb_encode_string(stream, (pb_byte_t *) decode_method_buffer, encode_did_method_bytes_written);
}

bool encode_did_schema_cb(pb_ostream_t *stream, const pb_field_t *field, void * const *arg){
    (void) arg;
    (void) field;
    log_string(DEBUG, "encode_did_schema_cb", "error_message_buffer", error_message_buffer);


    return pb_encode_string(stream, (pb_byte_t *) decode_schema_buffer, encode_did_schema_bytes_written);
}

pb_callback_t pb_encode_did_id_cb;
pb_callback_t pb_encode_did_method_cb;
pb_callback_t pb_encode_did_schema_cb;

int did_request_encode(uint8_t *buffer, size_t buffer_size, iotaDoorLock_DIDRequest *message_ptr) {
    (void) buffer;
    (void) buffer_size;
    //char func_name[] = "did_request_encode";

    message_ptr->id = pb_did_id_cb;
    message_ptr->method = pb_did_method_cb;
    message_ptr->schema = pb_did_schema_cb;

    return 0;
}

int did_response_encode(uint8_t *buffer, size_t buffer_size, iotaDoorLock_DIDResponse *message_ptr) {
    char func_name[] = "did_response_encode";

    log_did_response(func_name, message_ptr);

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    bool encode_status = pb_encode(&stream, iotaDoorLock_DIDResponse_fields, message_ptr);
    size_t encode_message_length = stream.bytes_written;

    if (!encode_status) {
        log_message(FATAL, func_name, "Encoding failed", (char *) PB_GET_ERROR(&stream));
        return 0;
    }else{
        log_encode_result(func_name, encode_status, encode_message_length);
    }

    return encode_message_length;
}

int access_status_encode(uint8_t *buffer, size_t buffer_size, iotaDoorLock_AccessStatus *message_ptr) {
    char func_name[] = "access_status_encode";

    log_message(DEBUG, func_name, "Encode", "Encoding access status...");
    log_access_status(func_name, message_ptr);

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    bool encode_status = pb_encode(&stream, iotaDoorLock_AccessStatus_fields, message_ptr);
    size_t encode_message_length = stream.bytes_written;

    if (!encode_status) {
        log_message(FATAL, func_name, "Encoding failed", (char *) PB_GET_ERROR(&stream));
        return 0;
    }else{
        log_encode_result(func_name, encode_status, encode_message_length);
    }

    return encode_message_length;
}

bool encode_error_message_cb(pb_ostream_t *stream, const pb_field_t *field, void * const *arg){
    (void) arg;
    (void) field;
    log_string(DEBUG, "error_response_encode_message_cb", "error_message_buffer", error_message_buffer);


    return pb_encode_string(stream, (pb_byte_t *) error_message_buffer, error_message_buffer_written_size);
}

int error_response_encode(uint8_t *buffer, size_t buffer_size, iotaDoorLock_ErrorResponse *message_ptr){
    char func_name[] = "error_response_encode";

    log_message(DEBUG, func_name, "Encode", "Encoding error response...");
    log_int(DEBUG, func_name, "error_time", message_ptr->time);

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    bool encode_status = pb_encode(&stream, iotaDoorLock_AccessStatus_fields, message_ptr);
    size_t encode_message_length = stream.bytes_written;

    if (!encode_status) {
        log_message(FATAL, func_name, "Encoding failed", (char *) PB_GET_ERROR(&stream));
        return 0;
    }else{
        log_encode_result(func_name, encode_status, encode_message_length);
    }

    return encode_message_length;
}