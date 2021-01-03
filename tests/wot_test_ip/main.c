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

static ipv6_addr_t *get_base_ip_address(void)
{
    const int MAX_ADRESSES = 5;
    netif_t *interface = netif_iter(NULL);
    ipv6_addr_t *local_address = NULL;
    ipv6_addr_t *ula_address = NULL;

    while (interface != NULL)
    {
        ipv6_addr_t adresses[MAX_ADRESSES];
        netif_get_opt(interface, NETOPT_IPV6_ADDR, 0, adresses, sizeof(adresses));
        for (int i = 0; i < MAX_ADRESSES; i++)
        {
            ipv6_addr_t *current_address = &adresses[i];

            if (current_address == NULL)
            {
                break;
            }
            if (ipv6_addr_is_global(current_address))
            {
                return current_address;
            }
            else if (ipv6_addr_is_unique_local_unicast(current_address))
            {
                ula_address = current_address;
            }
            else if (ipv6_addr_is_link_local(current_address))
            {
                local_address = current_address;
            }
        }
        interface = netif_iter(interface);
    }

    if (ula_address != NULL)
    {
        return local_address;
    }
    else if (local_address != NULL)
    {
        return local_address;
    }
    return NULL;
}

int ip_test_cmd(int argc, char **argv)
{
    if (argc != 1)
    {
        (void)puts("Usage: iptest test");
        return 1;
    }
    if (strcmp(argv[1], "on") == 0)
    {
        char address_as_string[IPV6_ADDR_MAX_STR_LEN];
        ipv6_addr_t *base_ip_adress = get_base_ip_address();
        assert(base_ip_adress);
        ipv6_addr_to_str(address_as_string, base_ip_adress, sizeof(address_as_string));
        print_str(address_as_string);
        return 0;
    }

    return 1;
}

static const shell_command_t shell_commands[] = {
    {"iptest", "Control an LED.", ip_test_cmd},
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