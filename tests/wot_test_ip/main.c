#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "xtimer.h"
#include "shell.h"
#include "fmt.h"
#include "net/netif.h"
#include "net/ipv6/addr.h"
#include "net/gnrc/netif.h"

#define MAIN_QUEUE_SIZE (4)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static int get_base_ip_address(ipv6_addr_t *res) {
    const int MAX_ADRESSES_TO_CHECK = 5;
    netif_t* interface = NULL;
    ipv6_addr_t local_address = {{0}};
    ipv6_addr_t ula_address = {{0}};

    bool link_local_found = false;
    bool ula_found = false;

    while ((interface = netif_iter(interface)) != NULL) {
        ipv6_addr_t adresses[MAX_ADRESSES_TO_CHECK];
        netif_get_opt(interface, NETOPT_IPV6_ADDR, 0, adresses, sizeof(adresses));
        for (int i = 0; i < MAX_ADRESSES_TO_CHECK; i++)
        {
            ipv6_addr_t* current_address = &adresses[i];

            if (current_address == NULL) {
                break;
            }
            printf("Loop begin\n");
            if (ipv6_addr_is_global(current_address)) {
                ipv6_addr_print(current_address);
                printf("\n");
                memcpy(res, current_address, sizeof(ipv6_addr_t));
                return 0;
            }
            else if (ipv6_addr_is_unique_local_unicast(current_address)) {
                continue;
                printf("Unique local unicast: ");
                memcpy(&ula_address, current_address, sizeof(ipv6_addr_t));
                ula_found = true;
            }
            else if (ipv6_addr_is_link_local(current_address)) {
                printf("Link local address: ");
                memcpy(&local_address, current_address, sizeof(ipv6_addr_t));
                link_local_found = true;
            }
            ipv6_addr_print(current_address);
            printf("\n");
            printf("Loop end\n");
        }
    }

    

    if (ula_found) {
        memcpy(res, &ula_address, sizeof(ipv6_addr_t));
        return 0;
    }
    else if (link_local_found) {
        //puts("Hi.");
        memcpy(res, &local_address, sizeof(ipv6_addr_t));
        return 0;
    }
    return -1;
}


static int print_ip_addresses(void) 
{
    char address_as_string[IPV6_ADDR_MAX_STR_LEN];
    ipv6_addr_t base_ip_address = {0};
    if (get_base_ip_address(&base_ip_address) < 0) {
        return 1;
    }
    printf("ipv6_addr_print:\n");
    ipv6_addr_print(&base_ip_address);
    printf("\nipv6_addr_to_str:\n");
    ipv6_addr_to_str(address_as_string, &base_ip_address, sizeof(address_as_string));
    print_str(address_as_string);
    printf("\n"); 
    return 0;
}

int ip_test_cmd(int argc, char **argv)
{
    if (argc != 2)
    {
        (void)puts("Usage: iptest test");
        return 1;
    }
    if (strcmp(argv[1], "test") == 0) 
    {
        return print_ip_addresses();
    }

    return 1;
}

static const shell_command_t shell_commands[] = {
    {"iptest", "Get an IP address of the highest scope possible.", ip_test_cmd},
    {NULL, NULL, NULL}};

int main(void)
{
    xtimer_sleep(3);
    /* for the thread running the shell */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    print_ip_addresses();
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}