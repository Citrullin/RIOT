/*
 * Copyright (C) 2015 IOTA Foundation
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example application for a POSIX socket server with sensor data (Protobuf)
 *
 * @author      Philipp-Alexander Blum <philipp.blum@iota.org>
 *
 * @}
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

//IOTA
#include "iota/kerl.h"
#include "iota/conversion.h"
#include "iota/addresses.h"
#include "iota/transfers.h"

#include "shell.h"
#include "msg.h"

#include "pthread.h"

#define PORT 51037
#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

extern int iota_cmd(int argc, char **argv);

static const shell_command_t shell_commands[] = {
        { "iota", "send a tx", iota_cmd },
        { NULL, NULL, NULL }
};

int sock;

extern int init_mutex(void);

int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("IOTA RIOT Env sensor server");

    struct sockaddr_in6 server_addr = {.sin6_addr = IPV6_ADDR_UNSPECIFIED};
    memset( &server_addr, 0, sizeof(server_addr) );
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons( PORT );
    server_addr.sin6_scope_id = 0;

    if ( (sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0 ) {
        perror( "socket failed" );
    }

    iota_wallet_init();
    init_mutex();

    /* start shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
