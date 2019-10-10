#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "config.h"
#include "logging.h"

log_level_t selected_log_level = LOG_LEVEL;

char level_str[10] = "";

void get_log_level_string(log_level_t log_level){
    switch(log_level){
        case DEBUG:
            strncpy(level_str, "DEBUG", sizeof(level_str));
            break;
        case INFO:
            strncpy(level_str, "INFO", sizeof(level_str));
            break;
        case WARN:
            strncpy(level_str, "WARN", sizeof(level_str));
            break;
        case ERROR:
            strncpy(level_str, "ERROR", sizeof(level_str));
            break;
        case FATAL:
            strncpy(level_str, "FATAL", sizeof(level_str));
            break;
        case OFF:
            strncpy(level_str, "INFO", sizeof(level_str));
    }
}

void log_bool(log_level_t log_level, char *func_name, char *key, bool value){
    if(selected_log_level >= log_level){
        get_log_level_string(log_level);
        printf("%s | %s | bool | %s: ", level_str, func_name, key);
        if(value){
            printf("true");
        }else{
            printf("false");
        }
        printf("\n");
    }
}

void log_message(log_level_t log_level, char *func_name, char *key, char *value){
    if(selected_log_level >= log_level){
        get_log_level_string(log_level);
        printf("%s | %s | message | %s: %s\n", level_str, func_name, key, value);
    }
}

void log_int(log_level_t log_level, char *func_name, char *key, int value) {
    if(selected_log_level >= log_level){
        get_log_level_string(log_level);
        printf("%s | %s | int | %s: %i\n", level_str, func_name, key, value);
    }
}

void log_string(log_level_t log_level, char *func_name, char *key, char *value) {
    if(selected_log_level >= log_level){
        get_log_level_string(log_level);
        printf("%s | %s | string | %s: %s\n", level_str, func_name, key, value);
    }
}

void log_hex(log_level_t log_level, char *func_name, char *key, uint8_t value) {
    if(selected_log_level >= log_level) {
        get_log_level_string(log_level);
        printf("%s | %s | hex | %s: 0x%x\n", level_str, func_name, key, value);
    }
}

void log_hex_array(log_level_t log_level, char *func_name, char *key, uint8_t *value, size_t length) {
    if(selected_log_level >= log_level) {
        get_log_level_string(log_level);
        printf("%s | %s | hex_array | %s: [ ", level_str, func_name, key);
        for (unsigned int i = 0; i < length; i++) {
            if (i == 0) {
                printf("0x%x", value[i]);
            } else {
                printf(", 0x%x", value[i]);
            }
        }
        printf(" ]\n");
    }
}