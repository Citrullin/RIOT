#include "ftroika.h"
#include "iota/conversion.h"
#include "stdio.h"

#include "xtimer.h"

int BENCHMARK_TIMES = 1000;

int run_hashing(void){
    // "This is some text." in tryte chars
    const char tryte_chars_in[] = "CCWCXCGDEAXCGDEAGDCDADTCEAHDTCLDHDSA";

    trit_t trit_in[3 * (sizeof(tryte_chars_in) - 1)];

    chars_to_trits(tryte_chars_in, trit_in, (unsigned int) (sizeof(tryte_chars_in) - 1));

    uint8_t hash_trit_out[243];

    // Balanced ternary to unbalanced ternary
    for(int i = 0; i < sizeof(trit_in); i++){
        trit_in[i]++;
    }

    fTroika(hash_trit_out, (unsigned long long) 243, (const uint8_t *) trit_in, (unsigned long long) sizeof(trit_in));

    trit_t balanced_trits_out[243];
    // Unbalanced ternary to balanced ternary
    for(int i = 0; i < sizeof(hash_trit_out); i++){
        balanced_trits_out[i] = (uint8_t) (hash_trit_out[i] - 1);
    }

    /*
    char tryte_chars_hash_out[243/3];
    trits_to_chars(balanced_trits_out, tryte_chars_hash_out, 243);

    for(int i = 0; i < sizeof(tryte_chars_hash_out); i++){
        printf("%c", tryte_chars_hash_out[i]);
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

    printf("Hash ternary char text with Troika %i times took %li microseconds to execute \n", BENCHMARK_TIMES, time_taken);
}