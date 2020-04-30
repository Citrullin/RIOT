/*
 * Copyright (C) 2017 Freie Universität Berlin
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
 * @brief       Example for demonstrating SAUL and the SAUL registry
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include "atca.h"
#include "atca_params.h"
#include "shell.h"
#include "xtimer.h"

#define SHA256_HASH_SIZE (32)

int sha_cmd(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s [data]\n", argv[0]);
        return 1;
    }

    /*
    printf("String: %s\n", argv[1]);
    atcab_sha_start();
    uint8_t result[64];
    atcab_sha_end(result, (uint16_t) strlen(argv[1]), (uint8_t *) argv[1]);

    puts("Result: ");
    for(int i = 0; i < 32; i++){
        printf("%x", result[i]);
    }
    puts("\n");
     */

    atca_delay_us(100);
    uint8_t expected[] =
            { 0x36, 0x46, 0xEF, 0xD6, 0x27, 0x6C, 0x0D, 0xCB, 0x4B, 0x07, 0x73, 0x41,
              0x88, 0xF4, 0x17, 0xB4, 0x38, 0xAA, 0xCF, 0xC6, 0xAE, 0xEF, 0xFA, 0xBE,
              0xF3, 0xA8, 0x5D, 0x67, 0x42, 0x0D, 0xFE, 0xE5 };

    puts("Expected: ");
    for(int i = 0; i < 32; i++){
        printf("%02x", expected[i] & 0xff);
    }
    puts("\n");

    atcab_sha_start();
    uint8_t result[SHA256_HASH_SIZE];
    memset(result, 0, SHA256_HASH_SIZE);
    uint8_t teststring[] = "chili cheese fries";
    atcab_sha_end(result, sizeof(teststring) - 1, teststring);

    puts("Result: ");
    for(int i = 0; i < 32; i++){
        printf("%02x", result[i] & 0xff);
    }
    puts("\n");


    return 0;
}

static const shell_command_t shell_commands[] = {
        { "sha", "sha hashing", sha_cmd },
        { NULL, NULL, NULL }
};

int main(void)
{
    xtimer_sleep(3);
    puts("Welcome to RIOT!\n");
    puts("Type `help` for help\n");

    /*
    atcab_sha_start();
    uint8_t result[64];
    uint8_t teststring[] = "test";
    atcab_sha_end(result, sizeof(teststring) - 1, teststring);

    puts("Result: ");
    for(int i = 0; i < 32; i++){
        printf("%x", result[i] & 0xff);
    }
    puts("\n");
     */

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
