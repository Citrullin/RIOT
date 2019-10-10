//Std header
#include <stdio.h>
#include <string.h>

//Nanopb
#include "pb_common.h"
#include "pb_encode.h"

//Nanopb compiled
#include "proto_compiled/AccessStatus.pb.h"
#include "proto_compiled/DIDResponse.pb.h"

#include "config.h"
#include "logging.h"


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
        case iotaDoorLock_DIDResponse_Code_SUCCESSFUL_SEND:
            strncpy(did_response_code_name, "SUCCESSFUL_SEND", sizeof(did_response_code_name));
            break;
        case iotaDoorLock_DIDResponse_Code_SEND_ERROR:
            strncpy(did_response_code_name, "SEND_ERROR", sizeof(did_response_code_name));
            break;
    }

    log_string(DEBUG, func_name, "did_response", did_response_code_name);
}

int did_response_encode(uint8_t *buffer, size_t buffer_size, iotaDoorLock_DIDResponse *message_ptr) {
    char func_name[] = "did_response_encode";

    log_did_response(func_name, message_ptr);

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    bool encode_status = pb_encode(&stream, iotaDoorLock_DIDResponse_fields, message_ptr);
    size_t encode_message_length = stream.bytes_written;

    log_encode_result(func_name, encode_status, encode_message_length);

    if (!encode_status) {
        log_message(FATAL, func_name, "Encoding failed", (char *) PB_GET_ERROR(&stream));
        return 0;
    }

    return encode_message_length;
}

int access_status_encode(uint8_t *buffer, size_t buffer_size, iotaDoorLock_AccessStatus *message_ptr) {
    char func_name[] = "access_status_encode";

    log_access_status(func_name, message_ptr);

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);

    bool encode_status = pb_encode(&stream, iotaDoorLock_AccessStatus_fields, message_ptr);
    size_t encode_message_length = stream.bytes_written;

    log_encode_result(func_name, encode_status, encode_message_length);
    if (!encode_status) {
        log_message(FATAL, func_name, "Encoding failed", (char *) PB_GET_ERROR(&stream));
        return 0;
    }

    return encode_message_length;
}