#ifndef IOTA_DOOF_LOCK_LOGGING_H
#define IOTA_DOOF_LOCK_LOGGING_H

#include <stdbool.h>

typedef enum {
    DEBUG = 5,
    INFO = 4,
    WARN = 3,
    ERROR = 2,
    FATAL = 1,
    OFF = 0
} log_level_t;

/**
 * Log structure:
 * LEVEL | FUNCTION_NAME | TYPE | KEY: VALUE
 *
 * LEVEL => ERROR, INFO, DEBUG, FATAL, WARN
 * FUNCTION_NAME => the string name of the calling function
 * TYPE => string, bool, message etc.
 * KEY => variable name or struct
 */

void log_int(log_level_t log_level, char *func_name, char *key, int value);
void log_string(log_level_t log_level, char *func_name, char *key, char *value);
void log_bool(log_level_t log_level, char *func_name, char *key, bool value);
void log_message(log_level_t log_level, char *func_name, char *key, char *value);
void log_hex(log_level_t log_level, char *func_name, char *key, uint8_t value);
void log_hex_array(log_level_t log_level, char *func_name, char *key, uint8_t *value, size_t length);

#endif //IOTA_DOOF_LOCK_LOGGING_H
