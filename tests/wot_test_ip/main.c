#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "xtimer.h"
#include "shell.h"
#include "fmt.h"
#include "net/netif.h"
#include "net/ipv6/addr.h"

#define MAIN_QUEUE_SIZE (4)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static ipv6_addr_t *get_base_ip_address(void){
    const int MAX_ADRESSES_TO_CHECK = 5;
    netif_t* interface = NULL;
    ipv6_addr_t* local_address = NULL;
    ipv6_addr_t* ula_address = NULL;

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
                printf("Global address: ");
                ipv6_addr_print(current_address);
                printf("\n");
                printf("Loop end\n");
                return current_address;
            }
            else if (ipv6_addr_is_unique_local_unicast(current_address)) {
                printf("Unique local unicast: ");
                ula_address = current_address;
            }
            else if (ipv6_addr_is_link_local(current_address)) {
                printf("Link local address: ");
                local_address = current_address;
            }
            ipv6_addr_print(current_address);
            printf("\n");
            printf("Loop end\n");
        }
    }

    if (ula_address != NULL) {
        return local_address;
        return 0;
    }
    else if (local_address != NULL) {
        return local_address;
    }
    return NULL;
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
        char address_as_string[IPV6_ADDR_MAX_STR_LEN];
        ipv6_addr_t *base_ip_adress = get_base_ip_address();
        assert(base_ip_adress);
        printf("ipv6_addr_print:\n");
        ipv6_addr_print(base_ip_adress);
        printf("\nipv6_addr_to_str:\n");
        ipv6_addr_to_str(address_as_string, base_ip_adress, sizeof(address_as_string));
        print_str(address_as_string);
        printf("\n");
        return 0;
    }

    return 1;
}

static const shell_command_t shell_commands[] = {
    {"iptest", "Get an IP address of the highest order possible.", ip_test_cmd},
    {NULL, NULL, NULL}};

int main(void)
{
    xtimer_sleep(3);
    /* for the thread running the shell */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}