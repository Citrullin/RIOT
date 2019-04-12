/**
 * File is depending on the OS. Replace if you use another OS.
 */

#include <stdio.h>
#include <string.h>
#include "xtimer.h"

#include "pthread.h"

//OS
#include <phydat.h>

//IOTA
extern void *run_send_thread(void *args);

uint64_t start_time;
pthread_t server_thread;

static void shell_send_cmd(void *args) {

    if (server_thread > 0) {
        puts("Server is already running.");
    } else {
        server_thread = pthread_create(&server_thread, NULL, &run_send_thread, args);
    }
}

int iota_cmd(int argc, char **argv) {
    int is_send = strcmp(argv[1], "send");

    if (argc < 1 || (is_send != 0) ) {
        printf("usage: %s [sends]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "send") == 0) {
        shell_send_cmd(argv[2]);
        return 0;
    }

    return 0;
}