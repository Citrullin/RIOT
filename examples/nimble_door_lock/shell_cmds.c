/**
 * File is depending on the OS. Replace if you use another OS.
 */

#include <stdio.h>
#include <string.h>
#include "xtimer.h"

#include "pthread.h"

#include "config.h"

//Server
extern void *run_server_thread(void *args);
extern void server_stop(void);
extern void server_status(void);
extern bool is_server_running(void);
extern char error_message_buffer[ERROR_BUFFER_SIZE];
extern uint32_t error_message_buffer_written_size;

uint64_t start_time;
pthread_t server_thread;

static void shell_start_server_cmd(void *args) {
    /**
     * POSIX Thread create command.
     * Runs on every OS, but the thread does not have a name in RIOT OS.
     * The thread is called pthread when using the ps command
     */
    server_thread = pthread_create(&server_thread, NULL, &run_server_thread, args);
}

static void shell_stop_server_cmd(void *args){
    /**
     * Workaround for RIOT OS. RIOT does not support pthread_cancel.
     * This variable is set to false which ends the loop in server_start_listening.
     * pthread_exit and close is executed after the server_start_listening loop,
     * so that the thread is closing itself. This should also work in other POSIX compatible OS.
     */
    (void) args;
    server_stop();
}

static void shell_did_request_example_cmd(void *args){
    (void) args;

}

static void shell_error_message_cmd(int argc, char **args){
    int is_message = strcmp(args[1], "message");

    if(argc < 2 || is_message != 0){
        printf("usage: server error [message]\n");
    }
    else{
        if(error_message_buffer_written_size == 0){
            printf("Error message not available.\n");
        }else{
            printf("Last error message: \n");
            printf("\t%s\n", error_message_buffer);
        }
    }
}

int server_cmd(int argc, char **argv) {
    int is_start = strcmp(argv[1], "start");
    int is_stop = strcmp(argv[1], "stop");
    int is_status = strcmp(argv[1], "status");
    int is_error = strcmp(argv[1], "error");

    if (argc < 1 || (is_start != 0 && is_stop != 0 && is_status != 0 && is_error != 0) ) {
        printf("usage: %s [start|stop|status|error]\n", argv[0]);
        return 1;
    }

    if(is_error == 0){
        shell_error_message_cmd(argc - 1, &argv[1]);
    }
    else if(is_start == 0 && is_server_running()){
        printf("Server is already running!\n");
    }
    else if (is_start == 0) {
        start_time = xtimer_now64().ticks64;
        shell_start_server_cmd(argv[2]);
        return 0;
    }

    if (is_stop == 0) {
        shell_stop_server_cmd(argv[2]);
        return 0;
    }

    if(is_status == 0) {
        if(is_server_running()){
            uint64_t now = xtimer_now64().ticks64;
            uint64_t time_running = now - start_time;
            printf("Server is running for %lu seconds\n", (uint32_t) (time_running / 1000000));
            server_status();
        }else{
            printf("Server is not running\n");
        }
        return 0;
    }

    return 0;
}