#include "mercha.h"
#include <omp.h>
#include <stdio.h>
void mercha(const uint8_t key[32], const uint8_t nonce[12], uint8_t *input, uint8_t *output, size_t length) {
    // double start_time = omp_get_wtime();
    chacha20_encrypt(key, nonce, 0, input, length);
    // double chacha_time = omp_get_wtime() - start_time;
    merkel_tree(input, output, length);
    // double merkel_time = omp_get_wtime() - start_time - chacha_time;
    // printf("Chacha20 encryption time: %f seconds\n", chacha_time);
    // printf("Merkle tree construction time: %f seconds\n", merkel_time);
}