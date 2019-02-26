#include "stdio.h"
#include "keccak/sha3.h"
#include "iota/conversion.h"

#include "xtimer.h"

int BENCHMARK_TIMES = 1000;

int run_hashing(void){
    // "This is some text." in tryte chars
    const unsigned char data[] = "This is some text.";
    unsigned char digest[384];

    SHA3_CTX ctx;
    sha3_384_Init(&ctx);
    sha3_Update(&ctx, data, sizeof(data) - 1);
    sha3_Final(&ctx, digest);

    /*
    for(int i = 0; i < 384/8; i++){
        printf("%02x", digest[i]);
    }
    printf("%c", '\n');
    */

    return 0;
}

int run_benchmark(void){
    for(int i = 0; i < BENCHMARK_TIMES; i++){
        run_hashing();
    }
    return 0;
}

int main(void){
    uint32_t start_time = xtimer_now_usec();
    run_benchmark();
    uint32_t end_time = xtimer_now_usec();

    uint32_t time_taken = end_time - start_time;

    printf("Hash text with Keccak %i times took %li microseconds to execute \n", BENCHMARK_TIMES, time_taken);
}