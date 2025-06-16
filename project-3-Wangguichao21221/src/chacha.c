#include "mercha.h"
#include <immintrin.h> // For SIMD intrinsics
#include <omp.h>       // For multithreading

#define ROTL32_SIMD(x, n) ({ \
    __m512i result; \
    __asm__ volatile ( \
        "vpslld %2, %1, %0\n\t"      \
        "vpsrld %3, %1, %%zmm1\n\t"  \
        "vporq %%zmm1, %0, %0"        \
        : "=v" (result)               \
        : "v" (x), "i" (n), "i" (32 - (n)) \
        : "%zmm1"                     \
    ); \
    result; \
})
#define chacha_quarter_round_SIMD(a, b, c, d) \
    __asm__ volatile ( \
        /* a += b */ \
        "vpaddd %1, %0, %0\n\t" \
        /* d ^= a */ \
        "vpxorq %0, %3, %3\n\t" \
        /* d <<<= 16 */ \
        "vprord $16, %3, %3\n\t" \
        /* c += d */ \
        "vpaddd %3, %2, %2\n\t" \
        /* b ^= c */ \
        "vpxorq %2, %1, %1\n\t" \
        /* b <<<= 12 */ \
        "vprord $20, %1, %1\n\t" \
        /* a += b */ \
        "vpaddd %1, %0, %0\n\t" \
        /* d ^= a */ \
        "vpxorq %0, %3, %3\n\t" \
        /* d <<<= 8 */ \
        "vprord $24, %3, %3\n\t" \
        /* c += d */ \
        "vpaddd %3, %2, %2\n\t" \
        /* b ^= c */ \
        "vpxorq %2, %1, %1\n\t" \
        /* b <<<= 7 */ \
        "vprord $25, %1, %1\n\t" \
        : "+v" (a), "+v" (b), "+v" (c), "+v" (d) \
        : \
        : "memory" \
    )

void chacha20_encrypt(const uint8_t key[32], const uint8_t nonce[12], uint32_t initial_counter, uint8_t *buffer, size_t length) {
    uint32_t key_words[8];
    uint32_t nonce_words[3];

    memcpy(key_words, key, 32);
    memcpy(nonce_words, nonce, 12);

    uint32_t state[16] = {
        0x61707865, 0x3320646e, 0x79622d32, 0x6b206574,             
        key_words[0], key_words[1], key_words[2], key_words[3],     
        key_words[4], key_words[5], key_words[6], key_words[7],     
        initial_counter,                                            
        nonce_words[0], nonce_words[1], nonce_words[2]              
    };

    const __m128i st0_3 = _mm_loadu_si128((__m128i*)state);
    const __m128i st4_7 = _mm_loadu_si128((__m128i*)(state + 4));
    const __m128i st8_11 = _mm_loadu_si128((__m128i*)(state + 8));
    const __m128i st12_15 = _mm_loadu_si128((__m128i*)(state + 12));
    
    const size_t total_blocks = (length + 63) / 64;
    #pragma omp parallel for schedule(static) 
    for (size_t block = 0; block < total_blocks; block += 4) {
        // 预取下一轮的数据到L1缓存
        if (block + 4 < total_blocks) {
            _mm_prefetch((char*)(buffer + (block + 4) * 64), _MM_HINT_T0);
            _mm_prefetch((char*)(buffer + (block + 5) * 64), _MM_HINT_T0);
            _mm_prefetch((char*)(buffer + (block + 6) * 64), _MM_HINT_T0);
            _mm_prefetch((char*)(buffer + (block + 7) * 64), _MM_HINT_T0);
        }
        
        // 预取更远的数据到L2缓存
        if (block + 8 < total_blocks) {
            _mm_prefetch((char*)(buffer + (block + 8) * 64), _MM_HINT_T0);
            _mm_prefetch((char*)(buffer + (block + 9) * 64), _MM_HINT_T0);
            _mm_prefetch((char*)(buffer + (block + 10) * 64), _MM_HINT_T0);
            _mm_prefetch((char*)(buffer + (block + 11) * 64), _MM_HINT_T0);
        }
        
        // 预取更远的数据到L3缓存（非临时）
        if (block + 12 < total_blocks) {
            _mm_prefetch((char*)(buffer + (block + 12) * 64), _MM_HINT_NTA);
            _mm_prefetch((char*)(buffer + (block + 13) * 64), _MM_HINT_NTA);
            _mm_prefetch((char*)(buffer + (block + 14) * 64), _MM_HINT_NTA);
            _mm_prefetch((char*)(buffer + (block + 15) * 64), _MM_HINT_NTA);
        }
        
        __m128i ws12_15_block0, ws12_15_block1,ws12_15_block2, ws12_15_block3;
        __m512i ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad;
        __m512i ls0_3_quad, ls4_7_quad, ls8_11_quad, ls12_15_quad;
        __m512i ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled;
        ws12_15_block0 = _mm_add_epi32(st12_15, _mm_set_epi32(0, 0, 0, block));
        ws12_15_block1 = _mm_add_epi32(st12_15, _mm_set_epi32(0, 0, 0, block + 1));
        ws12_15_block2 = _mm_add_epi32(st12_15, _mm_set_epi32(0, 0, 0, block + 2));
        ws12_15_block3 = _mm_add_epi32(st12_15, _mm_set_epi32(0, 0, 0, block + 3));
        ws0_3_quad = _mm512_setzero_si512();  // 初始化为0
        ws0_3_quad = _mm512_inserti32x4(ws0_3_quad, st0_3, 0);  // 插入到位置0 (bits 0-127)
        ws0_3_quad = _mm512_inserti32x4(ws0_3_quad, st0_3, 1);  // 插入到位置1 (bits 128-255)
        ws0_3_quad = _mm512_inserti32x4(ws0_3_quad, st0_3, 2);  // 插入到位置2 (bits 256-383)
        ws0_3_quad = _mm512_inserti32x4(ws0_3_quad, st0_3, 3);  // 插入到位置3 (bits 384-511)
        ws4_7_quad = _mm512_setzero_si512();  // 初始化为0
        ws4_7_quad = _mm512_inserti32x4(ws4_7_quad, st4_7, 0);  // 插入到位置0 (bits 0-127)
        ws4_7_quad = _mm512_inserti32x4(ws4_7_quad, st4_7, 1);  // 插入到位置1 (bits 128-255)
        ws4_7_quad = _mm512_inserti32x4(ws4_7_quad, st4_7, 2);  // 插入到位置2 (bits 256-383)
        ws4_7_quad = _mm512_inserti32x4(ws4_7_quad, st4_7, 3);  // 插入到位置3 (bits 384-511)
        ws8_11_quad = _mm512_setzero_si512();  // 初始化为0
        ws8_11_quad = _mm512_inserti32x4(ws8_11_quad, st8_11, 0);  // 插入到位置0 (bits 0-127)
        ws8_11_quad = _mm512_inserti32x4(ws8_11_quad, st8_11, 1);  // 插入到位置1 (bits 128-255)
        ws8_11_quad = _mm512_inserti32x4(ws8_11_quad, st8_11, 2);  // 插入到位置2 (bits 256-383)
        ws8_11_quad = _mm512_inserti32x4(ws8_11_quad, st8_11, 3);  // 插入到位置3 (bits 384-511)
        ws12_15_quad = _mm512_setzero_si512();  // 初始化为0
        ws12_15_quad = _mm512_inserti32x4(ws12_15_quad, ws12_15_block0, 0);  // 插入到位置0 (bits 0-127)
        ws12_15_quad = _mm512_inserti32x4(ws12_15_quad, ws12_15_block1, 1);  // 插入到位置1 (bits 128-255)
        ws12_15_quad = _mm512_inserti32x4(ws12_15_quad, ws12_15_block2, 2);  // 插入到位置2 (bits 256-383) 
        ws12_15_quad = _mm512_inserti32x4(ws12_15_quad, ws12_15_block3, 3);  // 插入到位置3 (bits 384-511)
        ls0_3_quad = ws0_3_quad;
        ls4_7_quad = ws4_7_quad;
        ls8_11_quad = ws8_11_quad;
        ls12_15_quad = ws12_15_quad;

            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
        ws0_3_quad = _mm512_add_epi32(ws0_3_quad, ls0_3_quad);
        ws4_7_quad = _mm512_add_epi32(ws4_7_quad, ls4_7_quad);
        ws8_11_quad = _mm512_add_epi32(ws8_11_quad, ls8_11_quad);
        ws12_15_quad = _mm512_add_epi32(ws12_15_quad, ls12_15_quad);
        uint8_t * block0_ptr = buffer + block * 64;
        uint8_t * block1_ptr = buffer + (block + 1) * 64;
        uint8_t * block2_ptr = buffer + (block + 2) * 64;
        uint8_t * block3_ptr = buffer + (block + 3) * 64;
        
        // 预取当前块的数据到L1缓存（立即需要）
        _mm_prefetch((char*)block0_ptr, _MM_HINT_T0);
        _mm_prefetch((char*)(block0_ptr + 32), _MM_HINT_T0);
        _mm_prefetch((char*)block1_ptr, _MM_HINT_T0);
        _mm_prefetch((char*)(block1_ptr + 32), _MM_HINT_T0);
        _mm_prefetch((char*)block2_ptr, _MM_HINT_T0);
        _mm_prefetch((char*)(block2_ptr + 32), _MM_HINT_T0);
        _mm_prefetch((char*)block3_ptr, _MM_HINT_T0);
        _mm_prefetch((char*)(block3_ptr + 32), _MM_HINT_T0);
        
        __m512i input_data1 =  _mm512_setzero_si512();
        __m512i input_data2 =  _mm512_setzero_si512();
        __m512i input_data3 =  _mm512_setzero_si512();
        __m512i input_data4 =  _mm512_setzero_si512();
        input_data1 = _mm512_inserti32x4(input_data1, _mm_loadu_si128((__m128i*)(block0_ptr)), 0);
        input_data2 = _mm512_inserti32x4(input_data2, _mm_loadu_si128((__m128i*)(block0_ptr + 16)), 0);
        input_data3 = _mm512_inserti32x4(input_data3, _mm_loadu_si128((__m128i*)(block0_ptr + 32)), 0);
        input_data4 = _mm512_inserti32x4(input_data4, _mm_loadu_si128((__m128i*)(block0_ptr + 48)), 0);

        input_data1 = _mm512_inserti32x4(input_data1, _mm_loadu_si128((__m128i*)(block1_ptr)), 1);
        input_data2 = _mm512_inserti32x4(input_data2, _mm_loadu_si128((__m128i*)(block1_ptr + 16)), 1);
        input_data3 = _mm512_inserti32x4(input_data3, _mm_loadu_si128((__m128i*)(block1_ptr + 32)), 1);
        input_data4 = _mm512_inserti32x4(input_data4, _mm_loadu_si128((__m128i*)(block1_ptr + 48)), 1);

        input_data1 = _mm512_inserti32x4(input_data1, _mm_loadu_si128((__m128i*)(block2_ptr)), 2);
        input_data2 = _mm512_inserti32x4(input_data2, _mm_loadu_si128((__m128i*)(block2_ptr + 16)), 2);
        input_data3 = _mm512_inserti32x4(input_data3, _mm_loadu_si128((__m128i*)(block2_ptr + 32)), 2);
        input_data4 = _mm512_inserti32x4(input_data4, _mm_loadu_si128((__m128i*)(block2_ptr + 48)), 2);

        input_data1 = _mm512_inserti32x4(input_data1, _mm_loadu_si128((__m128i*)(block3_ptr)), 3);
        input_data2 = _mm512_inserti32x4(input_data2, _mm_loadu_si128((__m128i*)(block3_ptr + 16)), 3);
        input_data3 = _mm512_inserti32x4(input_data3, _mm_loadu_si128((__m128i*)(block3_ptr + 32)), 3);
        input_data4 = _mm512_inserti32x4(input_data4, _mm_loadu_si128((__m128i*)(block3_ptr + 48)), 3);
        
        // XOR操作（向量化）
        __m512i output_data1 = _mm512_xor_si512(input_data1, ws0_3_quad);
        __m512i output_data2 = _mm512_xor_si512(input_data2, ws4_7_quad);
        __m512i output_data3 = _mm512_xor_si512(input_data3, ws8_11_quad);
        __m512i output_data4 = _mm512_xor_si512(input_data4, ws12_15_quad);

        _mm_storeu_si128((__m128i*)(block0_ptr), _mm512_extracti32x4_epi32(output_data1, 0));
        _mm_storeu_si128((__m128i*)(block0_ptr + 16), _mm512_extracti32x4_epi32(output_data2, 0));
        _mm_storeu_si128((__m128i*)(block0_ptr + 32), _mm512_extracti32x4_epi32(output_data3, 0));
        _mm_storeu_si128((__m128i*)(block0_ptr + 48), _mm512_extracti32x4_epi32(output_data4, 0));


        _mm_storeu_si128((__m128i*)(block1_ptr), _mm512_extracti32x4_epi32(output_data1, 1));
        _mm_storeu_si128((__m128i*)(block1_ptr + 16), _mm512_extracti32x4_epi32(output_data2, 1));
        _mm_storeu_si128((__m128i*)(block1_ptr + 32), _mm512_extracti32x4_epi32(output_data3, 1));
        _mm_storeu_si128((__m128i*)(block1_ptr + 48), _mm512_extracti32x4_epi32(output_data4, 1));

        _mm_storeu_si128((__m128i*)(block2_ptr), _mm512_extracti32x4_epi32(output_data1, 2));
        _mm_storeu_si128((__m128i*)(block2_ptr + 16), _mm512_extracti32x4_epi32(output_data2, 2));
        _mm_storeu_si128((__m128i*)(block2_ptr + 32), _mm512_extracti32x4_epi32(output_data3, 2));
        _mm_storeu_si128((__m128i*)(block2_ptr + 48), _mm512_extracti32x4_epi32(output_data4, 2));

        _mm_storeu_si128((__m128i*)(block3_ptr), _mm512_extracti32x4_epi32(output_data1, 3));
        _mm_storeu_si128((__m128i*)(block3_ptr + 16), _mm512_extracti32x4_epi32(output_data2, 3));
        _mm_storeu_si128((__m128i*)(block3_ptr + 32), _mm512_extracti32x4_epi32(output_data3, 3));
        _mm_storeu_si128((__m128i*)(block3_ptr + 48), _mm512_extracti32x4_epi32(output_data4, 3));

        block += 4; 
        ws12_15_block0 = _mm_add_epi32(st12_15, _mm_set_epi32(0, 0, 0, block));
        ws12_15_block1 = _mm_add_epi32(st12_15, _mm_set_epi32(0, 0, 0, block + 1));
        ws12_15_block2 = _mm_add_epi32(st12_15, _mm_set_epi32(0, 0, 0, block + 2));
        ws12_15_block3 = _mm_add_epi32(st12_15, _mm_set_epi32(0, 0, 0, block + 3));
        ws0_3_quad = _mm512_setzero_si512();  // 初始化为0
        ws0_3_quad = _mm512_inserti32x4(ws0_3_quad, st0_3, 0);  // 插入到位置0 (bits 0-127)
        ws0_3_quad = _mm512_inserti32x4(ws0_3_quad, st0_3, 1);  // 插入到位置1 (bits 128-255)
        ws0_3_quad = _mm512_inserti32x4(ws0_3_quad, st0_3, 2);  // 插入到位置2 (bits 256-383)
        ws0_3_quad = _mm512_inserti32x4(ws0_3_quad, st0_3, 3);  // 插入到位置3 (bits 384-511)
        ws4_7_quad = _mm512_setzero_si512();  // 初始化为0
        ws4_7_quad = _mm512_inserti32x4(ws4_7_quad, st4_7, 0);  // 插入到位置0 (bits 0-127)
        ws4_7_quad = _mm512_inserti32x4(ws4_7_quad, st4_7, 1);  // 插入到位置1 (bits 128-255)
        ws4_7_quad = _mm512_inserti32x4(ws4_7_quad, st4_7, 2);  // 插入到位置2 (bits 256-383)
        ws4_7_quad = _mm512_inserti32x4(ws4_7_quad, st4_7, 3);  // 插入到位置3 (bits 384-511)
        ws8_11_quad = _mm512_setzero_si512();  // 初始化为0
        ws8_11_quad = _mm512_inserti32x4(ws8_11_quad, st8_11, 0);  // 插入到位置0 (bits 0-127)
        ws8_11_quad = _mm512_inserti32x4(ws8_11_quad, st8_11, 1);  // 插入到位置1 (bits 128-255)
        ws8_11_quad = _mm512_inserti32x4(ws8_11_quad, st8_11, 2);  // 插入到位置2 (bits 256-383)
        ws8_11_quad = _mm512_inserti32x4(ws8_11_quad, st8_11, 3);  // 插入到位置3 (bits 384-511)
        ws12_15_quad = _mm512_setzero_si512();  // 初始化为0
        ws12_15_quad = _mm512_inserti32x4(ws12_15_quad, ws12_15_block0, 0);  // 插入到位置0 (bits 0-127)
        ws12_15_quad = _mm512_inserti32x4(ws12_15_quad, ws12_15_block1, 1);  // 插入到位置1 (bits 128-255)
        ws12_15_quad = _mm512_inserti32x4(ws12_15_quad, ws12_15_block2, 2);  // 插入到位置2 (bits 256-383) 
        ws12_15_quad = _mm512_inserti32x4(ws12_15_quad, ws12_15_block3, 3);  // 插入到位置3 (bits 384-511)
        ls0_3_quad = ws0_3_quad;
        ls4_7_quad = ws4_7_quad;
        ls8_11_quad = ws8_11_quad;
        ls12_15_quad = ws12_15_quad;

            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_quad, ws8_11_quad, ws12_15_quad);
            ws4_7_shuffled = _mm512_shuffle_epi32(ws4_7_quad, _MM_SHUFFLE(0,3,2,1));
            ws8_11_shuffled = _mm512_shuffle_epi32(ws8_11_quad, _MM_SHUFFLE(1,0,3,2));
            ws12_15_shuffled = _mm512_shuffle_epi32(ws12_15_quad, _MM_SHUFFLE(2,1,0,3));
            chacha_quarter_round_SIMD(ws0_3_quad, ws4_7_shuffled, ws8_11_shuffled, ws12_15_shuffled);
            ws4_7_quad = _mm512_shuffle_epi32(ws4_7_shuffled, _MM_SHUFFLE(2,1,0,3));
            ws8_11_quad = _mm512_shuffle_epi32(ws8_11_shuffled, _MM_SHUFFLE(1,0,3,2));
            ws12_15_quad = _mm512_shuffle_epi32(ws12_15_shuffled, _MM_SHUFFLE(0,3,2,1));
        ws0_3_quad = _mm512_add_epi32(ws0_3_quad, ls0_3_quad);
        ws4_7_quad = _mm512_add_epi32(ws4_7_quad, ls4_7_quad);
        ws8_11_quad = _mm512_add_epi32(ws8_11_quad, ls8_11_quad);
        ws12_15_quad = _mm512_add_epi32(ws12_15_quad, ls12_15_quad);
        block0_ptr = buffer + block * 64;
        block1_ptr = buffer + (block + 1) * 64;
        block2_ptr = buffer + (block + 2) * 64;
        block3_ptr = buffer + (block + 3) * 64;
        
        // 预取当前块的数据到L1缓存（立即需要）
        _mm_prefetch((char*)block0_ptr, _MM_HINT_T0);
        _mm_prefetch((char*)(block0_ptr + 32), _MM_HINT_T0);
        _mm_prefetch((char*)block1_ptr, _MM_HINT_T0);
        _mm_prefetch((char*)(block1_ptr + 32), _MM_HINT_T0);
        _mm_prefetch((char*)block2_ptr, _MM_HINT_T0);
        _mm_prefetch((char*)(block2_ptr + 32), _MM_HINT_T0);
        _mm_prefetch((char*)block3_ptr, _MM_HINT_T0);
        _mm_prefetch((char*)(block3_ptr + 32), _MM_HINT_T0);
        
        input_data1 =  _mm512_setzero_si512();
        input_data2 =  _mm512_setzero_si512();
        input_data3 =  _mm512_setzero_si512();
        input_data4 =  _mm512_setzero_si512();
        input_data1 = _mm512_inserti32x4(input_data1, _mm_loadu_si128((__m128i*)(block0_ptr)), 0);
        input_data2 = _mm512_inserti32x4(input_data2, _mm_loadu_si128((__m128i*)(block0_ptr + 16)), 0);
        input_data3 = _mm512_inserti32x4(input_data3, _mm_loadu_si128((__m128i*)(block0_ptr + 32)), 0);
        input_data4 = _mm512_inserti32x4(input_data4, _mm_loadu_si128((__m128i*)(block0_ptr + 48)), 0);

        input_data1 = _mm512_inserti32x4(input_data1, _mm_loadu_si128((__m128i*)(block1_ptr)), 1);
        input_data2 = _mm512_inserti32x4(input_data2, _mm_loadu_si128((__m128i*)(block1_ptr + 16)), 1);
        input_data3 = _mm512_inserti32x4(input_data3, _mm_loadu_si128((__m128i*)(block1_ptr + 32)), 1);
        input_data4 = _mm512_inserti32x4(input_data4, _mm_loadu_si128((__m128i*)(block1_ptr + 48)), 1);

        input_data1 = _mm512_inserti32x4(input_data1, _mm_loadu_si128((__m128i*)(block2_ptr)), 2);
        input_data2 = _mm512_inserti32x4(input_data2, _mm_loadu_si128((__m128i*)(block2_ptr + 16)), 2);
        input_data3 = _mm512_inserti32x4(input_data3, _mm_loadu_si128((__m128i*)(block2_ptr + 32)), 2);
        input_data4 = _mm512_inserti32x4(input_data4, _mm_loadu_si128((__m128i*)(block2_ptr + 48)), 2);

        input_data1 = _mm512_inserti32x4(input_data1, _mm_loadu_si128((__m128i*)(block3_ptr)), 3);
        input_data2 = _mm512_inserti32x4(input_data2, _mm_loadu_si128((__m128i*)(block3_ptr + 16)), 3);
        input_data3 = _mm512_inserti32x4(input_data3, _mm_loadu_si128((__m128i*)(block3_ptr + 32)), 3);
        input_data4 = _mm512_inserti32x4(input_data4, _mm_loadu_si128((__m128i*)(block3_ptr + 48)), 3);
        
        // XOR操作（向量化）
        output_data1 = _mm512_xor_si512(input_data1, ws0_3_quad);
        output_data2 = _mm512_xor_si512(input_data2, ws4_7_quad);
        output_data3 = _mm512_xor_si512(input_data3, ws8_11_quad);
        output_data4 = _mm512_xor_si512(input_data4, ws12_15_quad);

        _mm_storeu_si128((__m128i*)(block0_ptr), _mm512_extracti32x4_epi32(output_data1, 0));
        _mm_storeu_si128((__m128i*)(block0_ptr + 16), _mm512_extracti32x4_epi32(output_data2, 0));
        _mm_storeu_si128((__m128i*)(block0_ptr + 32), _mm512_extracti32x4_epi32(output_data3, 0));
        _mm_storeu_si128((__m128i*)(block0_ptr + 48), _mm512_extracti32x4_epi32(output_data4, 0));


        _mm_storeu_si128((__m128i*)(block1_ptr), _mm512_extracti32x4_epi32(output_data1, 1));
        _mm_storeu_si128((__m128i*)(block1_ptr + 16), _mm512_extracti32x4_epi32(output_data2, 1));
        _mm_storeu_si128((__m128i*)(block1_ptr + 32), _mm512_extracti32x4_epi32(output_data3, 1));
        _mm_storeu_si128((__m128i*)(block1_ptr + 48), _mm512_extracti32x4_epi32(output_data4, 1));

        _mm_storeu_si128((__m128i*)(block2_ptr), _mm512_extracti32x4_epi32(output_data1, 2));
        _mm_storeu_si128((__m128i*)(block2_ptr + 16), _mm512_extracti32x4_epi32(output_data2, 2));
        _mm_storeu_si128((__m128i*)(block2_ptr + 32), _mm512_extracti32x4_epi32(output_data3, 2));
        _mm_storeu_si128((__m128i*)(block2_ptr + 48), _mm512_extracti32x4_epi32(output_data4, 2));

        _mm_storeu_si128((__m128i*)(block3_ptr), _mm512_extracti32x4_epi32(output_data1, 3));
        _mm_storeu_si128((__m128i*)(block3_ptr + 16), _mm512_extracti32x4_epi32(output_data2, 3));
        _mm_storeu_si128((__m128i*)(block3_ptr + 32), _mm512_extracti32x4_epi32(output_data3, 3));
        _mm_storeu_si128((__m128i*)(block3_ptr + 48), _mm512_extracti32x4_epi32(output_data4, 3));
    }
}