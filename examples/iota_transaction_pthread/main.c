/*
 * Copyright (C) 2018 Inria
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
 * @brief       Example demonstrating of transaction in IOTA with RIOT
 *https://github.com/embedded-iota/iota-c-light-wallet
 * @author      Philipp-Alexander Blum <philipp-blum@jakiku.de>
 *
 * @}
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "iota/kerl.h"
#include "iota/conversion.h"
#include "iota/addresses.h"
#include "iota/transfers.h"

#include "pthread.h"

typedef struct {
    iota_wallet_tx_output_t txs[1];
    uint32_t length;
} iota_txs_output_buffer_t;

typedef struct {
    iota_wallet_tx_input_t txs[1];
    uint32_t length;
} iota_txs_input_buffer_t;


//char transaction_chars[2674];
//char bundle_hash[81];
bool tx_receiver(iota_wallet_tx_object_t * tx_object){
    puts("\n");
    puts("Address: ");
    puts(tx_object->address);
    puts("Tag: ");
    puts(tx_object->tag);
    puts("Value: ");
    printf("\n%li\n", (long int) tx_object->value);
    puts("currentIndex:");
    printf("\n%li\n", (long int) tx_object->currentIndex);
    puts("lastIndex:");
    printf("\n%li\n", (long int) tx_object->lastIndex);
    puts("Signature: ");
    puts(tx_object->signatureMessageFragment);
    puts("\n\n");
    //puts("raw transaction data:\n");
    //iota_wallet_construct_raw_transaction_chars(transaction_chars,bundle_hash, tx_object);
    //puts(transaction_chars);

    return true;
}

bool bundle_receiver(char * hash){
    puts("HASH: ");
    puts("");
    //strcpy(bundle_hash, hash);
    for(int i = 0; i < 81; i++){
        printf("%c", hash[i]);
    }
    puts("\n\n");

    return true;
}

char seedChars[] = "KNZ9GKOZS9TLPXKBHYUVWBZWSIGYZYRULTNDEBIIFAJEOADHCEYEQJPNIATEORDVQUBLIIGGBISRNQDDH";

char address_from[81];
char address_to[81];
static pthread_mutex_t address_mutex = {};
static pthread_mutexattr_t address_mutex_attr = {};

static pthread_mutex_t seed_mutex = {};
static pthread_mutexattr_t seed_mutex_attr = {};

void clear_addresses(void){
    memset(address_from, '9', 81);
    memset(address_to, '9', 81);
}

bool print_status(iota_wallet_status_codes_t * status){
    puts("Returned status: ");
    switch(*status){
        case BUNDLE_CREATION_SUCCESS:
            puts("BUNDLE_CREATION_SUCCESS");
            break;
        case BUNDLE_CREATION_TRANSACTION_RECEIVER_ERROR:
            puts("BUNDLE_CREATION_TRANSACTION_RECEIVER_ERROR");
            break;
        case BUNDLE_CREATION_BUNDLE_RECEIVER_ERROR:
            puts("BUNDLE_CREATION_BUNDLE_RECEIVER_ERROR");
            break;
    }
    puts("\n");

    return true;
}

void * run_thread(void * args){
    (void) args;

    uint8_t security = 2;

    unsigned char seedBytes[48];
    chars_to_bytes(seedChars, seedBytes, 81);

    iota_txs_output_buffer_t output_buffer = {};
    iota_txs_input_buffer_t input_buffer = {};

    pthread_mutex_lock(&address_mutex);
    clear_addresses();

    pthread_mutex_lock(&seed_mutex);
    iota_wallet_get_address(seedChars, 0, (unsigned int) security, address_from);
    iota_wallet_get_address(seedChars, 1, (unsigned int) security, address_to);
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
    iota_wallet_status_codes_t status =
            iota_wallet_create_tx_bundle(&bundle_receiver, &tx_receiver, &bundle_description);

    print_status(&status);

    puts("Prepared Transfer.");

    puts("DONE.");

    return 0;
}

int init_mutex(void){
    pthread_mutex_init(&address_mutex, &address_mutex_attr);
    pthread_mutex_init(&seed_mutex, &seed_mutex_attr);

    return 1;
}

#define NUM_THREADS 1

pthread_t threads[NUM_THREADS];
int thread_args[NUM_THREADS];

//Fixme: It is not preparing a tx bundle
int main(void)
{
    puts("IOTA Wallet Application");
    puts("=====================================");

    iota_wallet_init();
    init_mutex();
    //create all threads one by one
    for(int i = 0;i<NUM_THREADS; i++){
        printf("IN MAIN: Creating thread %d.\n",i+1);
        threads[i] = (pthread_t) i;
        thread_args[i] = i;
        int result_code = pthread_create(&threads[i],NULL,&run_thread,&thread_args[i]);
        assert(!result_code);
    }

    //wait for each thread to complete
    for(int i = 0; i < NUM_THREADS; i++){
        int result_code=pthread_join(threads[i],NULL);
        assert(!result_code);
        printf("Thread %d has ended.\n",i+1);
    }

    //iota_wallet_init();
    //run_bundle_creation();

    return 0;
}
