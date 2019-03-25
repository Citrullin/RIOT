/**
 * File is depending on the OS. Replace if you use another OS.
 */

#include <stdio.h>
#include <string.h>
#include "xtimer.h"

#include "pthread.h"

//OS
#include <phydat.h>

//Server
extern void *run_server_thread(void *args);
extern void server_stop(void);
extern void server_status(void);
extern bool is_server_running(void);

uint64_t start_time;
pthread_t server_thread;

static void shell_start_server_cmd(void *args) {
    /**
     * POSIX Thread create command.
     * Runs on every OS, but the thread does not have a name in RIOT OS.
     * The thread is called pthread when using the ps command
     */
    if (server_thread > 0) {
        puts("Server is already running.");
    } else {
        server_thread = pthread_create(&server_thread, NULL, &run_server_thread, args);
    }
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

int server_cmd(int argc, char **argv) {
    int is_start = strcmp(argv[1], "start");
    int is_stop = strcmp(argv[1], "stop");
    int is_status = strcmp(argv[1], "status");

    if (argc < 1 || (is_start != 0 && is_stop != 0 && is_status != 0) ) {
        printf("usage: %s [start|stop|status]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "start") == 0) {
        start_time = xtimer_now64().ticks64;
        shell_start_server_cmd(argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "stop") == 0) {
        shell_stop_server_cmd(argv[2]);
        return 0;
    }

    if(strcmp(argv[1], "status") == 0) {
        if(is_server_running()){
            uint64_t now = xtimer_now64().ticks64;
            uint64_t time_running = now - start_time;
            printf("Server is running for %lu seconds\n", (uint32_t) (time_running / 1000000));
            server_status();
        }else{
            printf("Server is not running");
        }
        return 0;
    }

    return 0;
}