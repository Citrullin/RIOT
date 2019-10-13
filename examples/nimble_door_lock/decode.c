//Std header
#include <stdio.h>

//Nanpb
#include "pb_common.h"
#include "pb_decode.h"

//Nanopb compiled
#include "proto_compiled/DIDRequest.pb.h"

#include "config.h"
#include "logging.h"

pb_byte_t decode_schema_buffer[DECODE_SCHEMA_SIZE] = "";
pb_byte_t decode_method_buffer[DECODE_METHOD_SIZE] = "";
pb_byte_t decode_id_buffer[DECODE_ID_SIZE] = "";

char decode_error_message_buffer[ERROR_BUFFER_SIZE];

void clear_decode_schema_buffer(void){
    memset(decode_schema_buffer, '\0', DECODE_SCHEMA_SIZE);
}

void clear_decode_method_buffer(void){
    memset(decode_method_buffer, '\0', DECODE_METHOD_SIZE);
}

void clear_decode_id_buffer(void){
    memset(decode_id_buffer, '\0', DECODE_ID_SIZE);
}

bool decode_did_schema_cb(pb_istream_t *stream, const pb_field_t *field, void **arg){
    (void) arg;

    if(field->data_size >= DECODE_SCHEMA_SIZE){
        log_message(FATAL, "decode_schema_cb", "Decoding failed",
            "Schema data size in protobuf message is too large for the schema buffer.");
        strncpy(decode_error_message_buffer, "Schema data size in protobuf message is too large for the schema buffer.",
            sizeof(decode_error_message_buffer));
        return false;
    }else{
        clear_decode_schema_buffer();
        decode_schema_buffer[field->data_size + 1] = '\0';
        return pb_read(stream, decode_schema_buffer, field->data_size);
    }
};

bool decode_did_method_cb(pb_istream_t *stream, const pb_field_t *field, void **arg){
    (void) arg;

    if(field->data_size >= DECODE_SCHEMA_SIZE){
        log_message(FATAL, "decode_method_cb", "Decoding failed",
                    "Method data size in protobuf message is too large for the schema buffer.");
        return false;
    }else{
        clear_decode_method_buffer();
        decode_method_buffer[field->data_size + 1] = '\0';
        return pb_read(stream, decode_method_buffer, field->data_size);
    }
};

bool decode_did_id_cb(pb_istream_t *stream, const pb_field_t *field, void **arg){
    (void) arg;

    if(field->data_size >= DECODE_SCHEMA_SIZE){
        log_message(FATAL, "decode_id_cb", "Decoding failed",
                    "ID data size in protobuf message is too large for the schema buffer.");
        return false;
    }else{
        clear_decode_id_buffer();
        decode_id_buffer[field->data_size + 1] = '\0';
        return pb_read(stream, decode_id_buffer, field->data_size);
    }
};



void log_did_buffers(char *func_name){
    log_string(DEBUG, func_name, "decode_schema_buffer", (char *) decode_schema_buffer);
    log_string(DEBUG, func_name, "decode_method_buffer", (char *) decode_method_buffer);
    log_string(DEBUG, func_name, "decode_id_buffer", (char *) decode_id_buffer);
}

char * decode_read_error_message(void){
    return decode_error_message_buffer;
}

bool did_request_decode(iotaDoorLock_DIDRequest *message_ptr, uint8_t *encoded_msg_ptr, size_t decoded_msg_size){
    char func_name[] = "did_request_decode";
    pb_istream_t stream = pb_istream_from_buffer(encoded_msg_ptr, decoded_msg_size);

    bool decode_status = pb_decode(&stream, iotaDoorLock_DIDRequest_fields, message_ptr);

    if (!decode_status) {
        log_message(FATAL, func_name, "Decoding failed", (char *) PB_GET_ERROR(&stream));
        strncpy(decode_error_message_buffer, (char *) PB_GET_ERROR(&stream), sizeof(decode_error_message_buffer));
    }

    log_did_buffers(func_name);

    return decode_status;
}