/*
 * Copyright (C) 2018 OTA keys S.A.
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
 * @brief       File system usage example application
 *
 * @author      Vincent Dupont <vincent@otakeys.com>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "shell.h"
#include "board.h" /* MTD_0 is defined in board.h */

#include "pulse_counter.h"
#include "xtimer.h"

/* Flash mount point */
#define FLASH_MOUNT_POINT   "/sda"

/* In this example, MTD_0 is used as mtd interface for littlefs or spiffs */
/* littlefs and spiffs basic usage are shown */
#ifdef MTD_0
/* File system descriptor initialization */
#if defined(MODULE_LITTLEFS)
/* include file system header for driver */
#include "fs/littlefs_fs.h"

/* file system specific descriptor
 * for littlefs, some fields can be tweaked to define the size
 * of the partition, see header documentation.
 * In this example, default behavior will be used, i.e. the entire
 * memory will be used (parameters come from mtd) */
static littlefs_desc_t fs_desc = {
    .lock = MUTEX_INIT,
};

/* littlefs file system driver will be used */
#define FS_DRIVER littlefs_file_system

#elif defined(MODULE_SPIFFS)
/* include file system header */
#include "fs/spiffs_fs.h"

/* file system specific descriptor
 * as for littlefs, some fields can be changed if needed,
 * this example focus on basic usage, i.e. entire memory used */
static spiffs_desc_t fs_desc = {
    .lock = MUTEX_INIT,
};

/* spiffs driver will be used */
#define FS_DRIVER spiffs_file_system
#endif

/* this structure defines the vfs mount point:
 *  - fs field is set to the file system driver
 *  - mount_point field is the mount point name
 *  - private_data depends on the underlying file system. For both spiffs and
 *  littlefs, it needs to be a pointer to the file system descriptor */
static vfs_mount_t flash_mount = {
    .fs = &FS_DRIVER,
    .mount_point = FLASH_MOUNT_POINT,
    .private_data = &fs_desc,
};
#endif /* MTD_0 */

/* constfs example */
#include "fs/constfs.h"

#define HELLO_WORLD_CONTENT "Hello World!\n"
#define HELLO_RIOT_CONTENT  "Hello RIOT!\n"

/* this defines two const files in the constfs */
static constfs_file_t constfs_files[] = {
    {
        .path = "/hello-world",
        .size = sizeof(HELLO_WORLD_CONTENT),
        .data = (const uint8_t *)HELLO_WORLD_CONTENT,
    },
    {
        .path = "/hello-riot",
        .size = sizeof(HELLO_RIOT_CONTENT),
        .data = (const uint8_t *)HELLO_RIOT_CONTENT,
    }
};

/* this is the constfs specific descriptor */
static constfs_t constfs_desc = {
    .nfiles = ARRAY_SIZE(constfs_files),
    .files = constfs_files,
};

/* constfs mount point, as for previous example, it needs a file system driver,
 * a mount point and private_data as a pointer to the constfs descriptor */
static vfs_mount_t const_mount = {
    .fs = &constfs_file_system,
    .mount_point = "/const",
    .private_data = &constfs_desc,
};

/* Command handlers */
static int _mount(int argc, char **argv)
{
    (void)argc;
    (void)argv;
#if defined(MTD_0) && (defined(MODULE_SPIFFS) || defined(MODULE_LITTLEFS))
    int res = vfs_mount(&flash_mount);
    if (res < 0) {
        printf("Error while mounting %s...try format\n", FLASH_MOUNT_POINT);
        return 1;
    }

    printf("%s successfully mounted\n", FLASH_MOUNT_POINT);
    return 0;
#else
    puts("No external flash file system selected");
    return 1;
#endif
}

static int _format(int argc, char **argv)
{
    (void)argc;
    (void)argv;
#if defined(MTD_0) && (defined(MODULE_SPIFFS) || defined(MODULE_LITTLEFS))
    int res = vfs_format(&flash_mount);
    if (res < 0) {
        printf("Error while formatting %s\n", FLASH_MOUNT_POINT);
        return 1;
    }

    printf("%s successfully formatted\n", FLASH_MOUNT_POINT);
    return 0;
#else
    puts("No external flash file system selected");
    return 1;
#endif
}

static int _umount(int argc, char **argv)
{
    (void)argc;
    (void)argv;
#if defined(MTD_0) && (defined(MODULE_SPIFFS) || defined(MODULE_LITTLEFS))
    int res = vfs_umount(&flash_mount);
    if (res < 0) {
        printf("Error while unmounting %s\n", FLASH_MOUNT_POINT);
        return 1;
    }

    printf("%s successfully unmounted\n", FLASH_MOUNT_POINT);
    return 0;
#else
    puts("No external flash file system selected");
    return 1;
#endif
}

static int _cat(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    /* With newlib, low-level syscalls are plugged to RIOT vfs
     * on native, open/read/write/close/... are plugged to RIOT vfs */
#ifdef MODULE_NEWLIB
    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        printf("file %s does not exist\n", argv[1]);
        return 1;
    }
    char c;
    while (fread(&c, 1, 1, f) != 0) {
        putchar(c);
    }
    fclose(f);
#else
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf("file %s does not exist\n", argv[1]);
        return 1;
    }
    char c;
    while (read(fd, &c, 1) != 0) {
        putchar(c);
    }
    close(fd);
#endif
    return 0;
}

static int _tee(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <file> <str>\n", argv[0]);
        return 1;
    }

#ifdef MODULE_NEWLIB
    FILE *f = fopen(argv[1], "w+");
    if (f == NULL) {
        printf("error while trying to create %s\n", argv[1]);
        return 1;
    }
    if (fwrite(argv[2], 1, strlen(argv[2]), f) != strlen(argv[2])) {
        puts("Error while writing");
    }
    fclose(f);
#else
    int fd = open(argv[1], O_RDWR | O_CREAT);
    if (fd < 0) {
        printf("error while trying to create %s\n", argv[1]);
        return 1;
    }
    if (write(fd, argv[2], strlen(argv[2])) != (ssize_t)strlen(argv[2])) {
        puts("Error while writing");
    }
    close(fd);
#endif
    return 0;
}

pulse_counter_t pulse_counter;
static pulse_counter_params_t pulse_counter_params = {
        .gpio = GPIO_PIN(PORT_A, 1),
        .gpio_flank = GPIO_BOTH,
};
int flow_meter_init(int argc, char **argv){
    (void) argv;
    if(argc != 2){
        printf("Usage: %s %s\n", argv[0], argv[1]);
        return 1;
    }

    if(pulse_counter_init(&pulse_counter, &pulse_counter_params) == -1){
        puts("Error while initializing flow_meter");
        return 1;
    }

    return 0;
}

int flow_meter_read(int argc, char **argv){
    if(argc != 3){
        printf("Usage: %s %s <seconds>\n", argv[0], argv[1]);
        return 1;
    }

    int seconds = atoi(argv[2]);
    printf("Will measure for %i seconds...\n", seconds);
    pulse_counter_reset(&pulse_counter);
    xtimer_sleep(seconds);
    int16_t pulses = pulse_counter_read_with_reset(&pulse_counter);
    //Liter/h
    int result = ((pulses / seconds) * 60 / 7.5);
    printf("Result: %i Liter/hour\n", result);

    return 0;
}

static int flow_meter(int argc, char **argv){
    if (argc > 3 || argc < 2) {
        printf("Usage: %s <command> (<value>)\n", argv[0]);
        return 1;
    }

    if(strcmp(argv[1], "init") == 0){
        return flow_meter_init(argc, argv);
    } else if(strcmp(argv[1], "read") == 0){
        return flow_meter_read(argc, argv);
    }

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "mount", "mount flash filesystem", _mount },
    { "format", "format flash file system", _format },
    { "umount", "unmount flash filesystem", _umount },
    { "cat", "print the content of a file", _cat },
    { "tee", "write a string in a file", _tee },
    { "flow_meter", "interact with the flow meter", flow_meter },
    { NULL, NULL, NULL }
};

int main(void)
{
#if defined(MTD_0) && (defined(MODULE_SPIFFS) || defined(MODULE_LITTLEFS))
    /* spiffs and littlefs need a mtd pointer
     * by default the whole memory is used */
    fs_desc.dev = MTD_0;
#endif
    int res = vfs_mount(&const_mount);
    if (res < 0) {
        puts("Error while mounting constfs");
    }
    else {
        puts("constfs mounted successfully");
    }

    res = vfs_mount(&flash_mount);
    if (res < 0) {
        puts("Error while mounting flash");
    }
    else {
        puts("flash mounted successfully");
    }

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
