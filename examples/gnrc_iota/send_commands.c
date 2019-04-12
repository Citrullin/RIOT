/**
 * This implementation should be C99 & POSIX compatible. Independent of the OS.
 * If not => create github issue
 * So, you should be able to use it in other POSIX compatible OS as well.
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include "pthread.h"

//IOTA
#include "iota/kerl.h"
#include "iota/conversion.h"
#include "iota/addresses.h"
#include "iota/transfers.h"

#include "proto_compiled/iota-transaction.pb.h"


//Todo: Use macros to generate debug code.
#define DEBUG_SERVER true

#define PORT 51037
#define MAXLINE 1000

extern int encode_iota_transaction_message(IotaTransactionMessage * message_ptr);

typedef struct {
    iota_wallet_tx_output_t txs[1];
    uint32_t length;
} iota_txs_output_buffer_t;

typedef struct {
    iota_wallet_tx_input_t txs[1];
    uint32_t length;
} iota_txs_input_buffer_t;

extern int sock;
int addr_len;
struct sockaddr_in6 client_addr;
struct sockaddr_in6 server_addr = {.sin6_addr = IPV6_ADDR_UNSPECIFIED};


IotaTransactionMessage iota_transaction_message;
iota_wallet_tx_object_t * wallet_tx_object;

int tx_receiver(iota_wallet_tx_object_t * tx_object){

    wallet_tx_object = tx_object;

    puts("\n");
    puts("Address: ");
    puts(tx_object->address);
    puts("Tag: ");
    puts(tx_object->tag);
    puts("Value: ");
    printf("\n%li\n", (long int) tx_object->value);
    puts("currentIndex:");
    printf("\n%li\n", tx_object->currentIndex);
    puts("lastIndex:");
    printf("\n%li\n", tx_object->lastIndex);
    puts("Signature: ");
    puts(tx_object->signatureMessageFragment);
    puts("\n\n");

    iota_transaction_message.value = tx_object->value;
    iota_transaction_message.currentIndex = tx_object->currentIndex;
    iota_transaction_message.lastIndex = tx_object->lastIndex;
    iota_transaction_message.timestamp = 0;

    encode_iota_transaction_message(&iota_transaction_message);

    return 1;
}

int bundle_receiver(char * hash){
    puts("HASH: ");
    puts("");

    for(int i = 0; i < 81; i++){
        printf("%c", hash[i]);
    }
    puts("\n\n");

    return 1;
}

char seedChars[] = "KNZ9GKOZS9TLPXKBHYUVWBZWSIGYZYRULTNDEBIIFAJEOADHCEYEQJPNIATEORDVQUBLIIGGBISRNQDDH";

char address_from[81];
char address_to[81];

static pthread_mutex_t address_mutex = {};
static pthread_mutexattr_t address_mutex_attr = {};

static pthread_mutex_t seed_mutex = {};
static pthread_mutexattr_t seed_mutex_attr = {};

int init_mutex(void){
    pthread_mutex_init(&address_mutex, &address_mutex_attr);
    pthread_mutex_init(&seed_mutex, &seed_mutex_attr);

    return 1;
}

void clear_addresses(void){
    memset(address_from, '9', 81);
    memset(address_to, '9', 81);
}

void *run_send_thread(void *args) {
    (void) args;

    unsigned char seedBytes[48];
    chars_to_bytes(seedChars, seedBytes, 81);

    iota_txs_output_buffer_t output_buffer = {};
    iota_txs_input_buffer_t input_buffer = {};

    pthread_mutex_lock(&address_mutex);
    clear_addresses();

    pthread_mutex_lock(&seed_mutex);
    iota_wallet_get_address(seedChars, 0, 2, address_from);
    iota_wallet_get_address(seedChars, 1, 2, address_to);
    pthread_mutex_unlock(&seed_mutex);

    //Alias for txs buffer
    iota_wallet_tx_output_t * txs_output = output_buffer.txs;
    iota_wallet_tx_input_t * txs_input = input_buffer.txs;

    puts("Create IOTA transactions bundle...");

    //Define output
    iota_wallet_tx_output_t * first_output = &txs_output[0];
    memcpy(first_output->address, address_to, 81);
    first_output->value = 10000;


    //Define the input array. Where the coins come from
    iota_wallet_tx_input_t * first_input = &txs_input[0];

    first_input->key_index = 0;
    first_input->value = 10000;
    memcpy(first_input->address, address_from, 81);

    pthread_mutex_unlock(&address_mutex);

    puts("Prepare transfer...");

    uint8_t security = 2;
    iota_wallet_bundle_description_t bundle_description = {};

    pthread_mutex_lock(&seed_mutex);
    puts("Copy Seed...");
    memcpy(bundle_description.seed, seedChars, 81);
    pthread_mutex_unlock(&seed_mutex);

    puts("Create bundle description...");
    bundle_description.security = security;
    bundle_description.output_txs = output_buffer.txs;
    bundle_description.output_txs_length = 1;
    bundle_description.input_txs = input_buffer.txs;
    bundle_description.input_txs_length = 1;
    bundle_description.timestamp = 0;

    puts("Create tx bundle...");
    iota_wallet_create_tx_bundle(&bundle_receiver, &tx_receiver, &bundle_description);

    puts("Prepared Transfer.");

    puts("DONE.");

    int value = 0;

    pthread_exit(&value);
    return 0;
}

#define NUM_THREADS 2

pthread_t threads[NUM_THREADS];
int thread_args[NUM_THREADS];