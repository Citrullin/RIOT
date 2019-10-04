/**
 * This implementation should be C99 & POSIX compatible. Independent of the OS.
 * If not => create github issue
 * So, you should be able to use it in other POSIX compatible OS as well.
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include "pthread.h"

#include "proto_compiled/DataRequest.pb.h"
#include "proto_compiled/DataResponse.pb.h"
#include "proto_compiled/FeatureResponse.pb.h"

/**
 * Log structure:
 * LEVEL | FUNCTION_NAME | TYPE | KEY: VALUE
 *
 * LEVEL => ERROR, INFO, DEBUG
 * FUNCTION_NAME => the string name of the calling function
 * TYPE => string, bool etc.
 * KEY => variable name or struct
 */

//Todo: Use macros to generate debug code.
#define DEBUG_SERVER true

uint32_t status_received_ddid_messages_successful = 0;
uint32_t status_error_count = 0;
uint32_t status_opened_door = 0;

uint8_t encode_buffer[500];
size_t encode_buffer_length;

void clear_encode_buffer(void) {
    memset(encode_buffer, 0, sizeof(encode_buffer));
    encode_buffer_length = 0;
}

#define RESPONSE_BUFFER_SIZE 500
uint8_t response_buffer[RESPONSE_BUFFER_SIZE];
size_t response_buffer_length;

void clear_response_buffer(void) {
    memset(response_buffer, 0, RESPONSE_BUFFER_SIZE);
    socket_buffer_length = 0;
}

//Logging
extern void log_int(char *level, char *func_name, char *key, int value);

extern void log_string(char *level, char *func_name, char *key, char *value);

extern void log_hex(char *level, char *func_name, char *key, uint8_t value);

extern void log_addr(char *level, char *func_name, char *key, struct sockaddr_in6 *client_addr);

extern void log_hex_array(char *level, char *func_name, char *key, uint8_t *value, size_t length);

//Encode
extern int env_sensor_data_response_encode(
        uint8_t *buffer, size_t buffer_size, environmentSensors_DataResponse *message_ptr);

extern int env_sensor_feature_response_encode(uint8_t *buffer, size_t buffer_size,
                                              environmentSensors_FeatureResponse *message_ptr);




//Decode
extern int env_sensor_data_request_decode(
        environmentSensors_DataRequest *message_ptr,
        uint8_t *encoded_msg_ptr, size_t decoded_msg_size);

extern int env_sensor_feature_request_decode(
        environmentSensors_FeatureResponse *message_ptr,
        uint8_t *encoded_msg_ptr, size_t decoded_msg_size);


typedef enum {
    FEATURE_REQUEST_CMD,
    FEATURE_RESPONSE_CMD,
    DATA_REQUEST_CMD,
    DATA_RESPONSE_CMD,
    ERROR_RESPONSE_CMD,
    SETUP_TEST_CMD,
    NONE_MESSAGE
} env_sensor_rpc_command_t;

env_sensor_rpc_command_t get_env_sensor_rpc_command(uint8_t type) {
    switch (type) {
        case 33:
            return FEATURE_REQUEST_CMD;
        case 34:
            return DATA_REQUEST_CMD;
        case 35:
            return DATA_RESPONSE_CMD;
        case 36:
            return FEATURE_RESPONSE_CMD;
        case 88:
            return SETUP_TEST_CMD;
        default:
            return NONE_MESSAGE;
    }
}

uint8_t get_env_sensor_rpc_command_byte(env_sensor_rpc_command_t command) {
    switch (command) {
        case FEATURE_REQUEST_CMD:
            return 33;
        case DATA_REQUEST_CMD:
            return 34;
        case DATA_RESPONSE_CMD:
            return 35;
        case FEATURE_RESPONSE_CMD:
            return 36;
        default:
            return 0;
    }
}

int get_env_sensor_rpc_command_name(char *result, env_sensor_rpc_command_t command) {
    switch (command) {
        case FEATURE_REQUEST_CMD:
            strcat(result, "FEATURE_REQUEST_CMD");
            break;
        case DATA_REQUEST_CMD:
            strcat(result, "DATA_REQUEST_CMD");
            break;
        case DATA_RESPONSE_CMD:
            strcat(result, "DATA_RESPONSE_CMD");
            break;
        case FEATURE_RESPONSE_CMD:
            strcat(result, "DATA_RESPONSE_CMD");
            break;
        default:
            strcat(result, "NONE_CMD");
    }

    return 0;
}

void send_buffer(int sock, struct sockaddr_in6 *client_addr_ptr) {
    if (DEBUG_SERVER) {
        log_addr("DEBUG", "send_buffer", "client_addr", client_addr_ptr);
    }

    status_send_packets += 1;
    sendto(
            sock, encode_buffer, encode_buffer_length, 0,
            (struct sockaddr *) client_addr_ptr, sizeof(struct sockaddr_in6));
}

void handle_env_sensor_feature_request(int sock, struct sockaddr_in6 *client_addr_ptr) {
    clear_encode_buffer();

    encode_buffer[0] = get_env_sensor_rpc_command_byte(FEATURE_RESPONSE_CMD);
    encode_buffer_length = 1;

    encode_buffer_length +=
            env_sensor_feature_response_encode(
                    &encode_buffer[1], sizeof(encode_buffer) - 1, &sensor_node_features);

    send_buffer(sock, client_addr_ptr);
}


extern void sensors_read_values(void);
extern environmentSensors_SingleDataPoint * sensors_get_temp_value(void);
//extern environmentSensors_SingleDataPoint * sensors_get_hum_value(void);

environmentSensors_DataResponse dataResponse;
void handle_env_sensor_data_request(int sock, struct sockaddr_in6 *client_addr_ptr) {
    clear_encode_buffer();
    sensors_read_values();

    encode_buffer[0] = get_env_sensor_rpc_command_byte(DATA_RESPONSE_CMD);
    encode_buffer_length = 1;

    environmentSensors_SingleDataPoint * temp_data_point = sensors_get_temp_value();
    //environmentSensors_SingleDataPoint * hum_data_point = sensors_get_hum_value();


    dataResponse.has_temperature = true;
    dataResponse.temperature = *temp_data_point;

    //dataResponse.has_humanity = true;
    //dataResponse.humanity = *hum_data_point;

    encode_buffer_length +=
            env_sensor_data_response_encode(
                    &encode_buffer[1], sizeof(encode_buffer) - 1, &dataResponse);

    send_buffer(sock, client_addr_ptr);
}

void add_incoming_message_to_status(env_sensor_rpc_command_t *command) {
    switch (*command) {
        case FEATURE_REQUEST_CMD:
            status_received_packets_successful += 1;
            break;
        default:
            status_received_packets_error += 1;
            break;
    }
}

void handle_incoming_message(int sock, struct sockaddr_in6 *client_addr_ptr) {
    char func_name[] = "handle_incoming_message";

    env_sensor_rpc_command_t message_command = get_env_sensor_rpc_command(socket_buffer[0]);

    if (DEBUG_SERVER) {
        log_hex("DEBUG", func_name, "hex_command", (uint8_t) socket_buffer[0]);
        char command_name[30] = "";
        get_env_sensor_rpc_command_name(command_name, message_command);
        log_string("DEBUG", func_name, "command_name", command_name);
        log_int("DEBUG", func_name, "client_port", ntohs(client_addr_ptr->sin6_port));
        log_addr("DEBUG", func_name, "client_addr", client_addr_ptr);
    }
    add_incoming_message_to_status(&message_command);

    switch (message_command) {
        case FEATURE_REQUEST_CMD:
            handle_env_sensor_feature_request(sock, client_addr_ptr);
            break;
        case DATA_REQUEST_CMD:
            handle_env_sensor_data_request(sock, client_addr_ptr);
            break;
        default:
            log_hex("ERROR", func_name, "hex_command", (uint8_t) socket_buffer[0]);
            log_addr("ERROR", func_name, "client_addr", client_addr_ptr);
            break;
    }
}

bool initialized_server;
bool server_is_running;
int server_socket;

extern void sensors_init(void);

void server_init(struct sockaddr_in6 *server_addr, size_t server_addr_size) {
    sensors_init();

    memset(server_addr, 0, server_addr_size);
    server_addr->sin6_family = AF_INET6;
    server_addr->sin6_port = htons(PORT);
    server_addr->sin6_scope_id = 0;

    if (DEBUG_SERVER) {
        log_addr("DEBUG", "server_init", "server_addr", server_addr);
        log_int("DEBUG", "server_init", "server_port", ntohs(server_addr->sin6_port));
    }

    if ((server_socket = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        log_int("ERROR", "server_init", "server_socket", server_socket);
    }

    if (bind(server_socket, (struct sockaddr *) server_addr, server_addr_size) < 0) {
        log_string("ERROR", "server_init", "bind_server_socket", "bind failed");
    }

    initialized_server = true;
    server_is_running = true;
}


void server_start_listening(void) {
    unsigned int client_addr_len = sizeof(struct sockaddr_in6);
    struct sockaddr_in6 client_addr;

    while (server_is_running) {
        clear_socket_buffer();
        int length = recvfrom(server_socket, socket_buffer, sizeof(socket_buffer) - 1, 0,
                              (struct sockaddr *) &client_addr,
                              &client_addr_len);

        if (length < 0) {
            log_int("ERROR", "server_start_listening", "recvfrom", length);
            break;
        }
        socket_buffer[length] = '\0';
        socket_buffer_length = length;

        if (server_is_running) {
            handle_incoming_message(server_socket, &client_addr);
        } else {
            clear_socket_buffer();
        }
    }
}

void server_stop(void) {
    server_is_running = false;
    status_received_packets_error = 0;
    status_received_packets_successful = 0;
    status_send_packets = 0;
}

void server_status(void) {
    //Todo: Rethink it. Maybe return a struct instead. Log instead in shell_cmds
    printf("Received %lu correct packets\n", status_received_packets_successful);
    printf("Received %lu incorrect packets\n", status_received_packets_error);
    printf("Send %lu packets\n", status_send_packets);
}

bool is_server_running(void) {
    return server_is_running;
}

void *run_server_thread(void *args) {
    (void) args;

    struct sockaddr_in6 server_addr = {.sin6_addr = IPV6_ADDR_UNSPECIFIED};
    server_init(&server_addr, sizeof(server_addr));
    server_start_listening();
    int value = 0;
    close(server_socket);
    pthread_exit(&value);
    return 0;
}