#include "mercha.h"
#include <omp.h>
#include <immintrin.h> /* header file for the SSE intrinsics */
#include <stdio.h>
#define ROTL32_SIMD(x, n) \
    (_mm512_or_si512(_mm512_slli_epi32((x), (n)), _mm512_srli_epi32((x), (32 - (n)))))
void merkel_tree(const uint8_t *input, uint8_t *output, size_t length){
    uint8_t *cur_buf = aligned_alloc(16,length); // 分配 16 字节对齐的内存
    uint8_t *prev_buf = aligned_alloc(16,length);
    memcpy(prev_buf, input, length);

    length /= 2;
    while (length >= 64) {
        if (length<=16){
            for (int i = 0; i < length / 64; i+=4) {
                // 预取下一轮的输入数据到L1缓存
                if (i + 4 < length / 64) {
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 4)) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 4) + 1) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 5)) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 5) + 1) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 6)) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 6) + 1) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 7)) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 7) + 1) * 64), _MM_HINT_T0);
                }
                
                // 预取更远的数据到L2缓存
                if (i + 8 < length / 64) {
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 8)) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 8) + 1) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 9)) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 9) + 1) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 10)) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 10) + 1) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 11)) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 11) + 1) * 64), _MM_HINT_T1);
                }
                
                // 预取输出缓冲区的位置
                _mm_prefetch((char*)(cur_buf + i * 64), _MM_HINT_T0);
                _mm_prefetch((char*)(cur_buf + (i + 1) * 64), _MM_HINT_T0);
                _mm_prefetch((char*)(cur_buf + (i + 2) * 64), _MM_HINT_T0);
                _mm_prefetch((char*)(cur_buf + (i + 3) * 64), _MM_HINT_T0);
                
                __m128i w1_03, w1_47, w2_03, w2_47,w2_74,w2_30,st0_3_first, st4_7_first, st8_11_first, st12_15_first,
                        st0_3_second, st4_7_second, st8_11_second, st12_15_second,st0_3_third, st4_7_third, st8_11_third, st12_15_third,
                        st0_3_fourth, st4_7_fourth, st8_11_fourth, st12_15_fourth;
                __m512i st0_3_quad, st4_7_quad, st8_11_quad, st12_15_quad;
                uint8_t *block1 = prev_buf + (2 * i) * 64;
                uint8_t *block2 = prev_buf + (2 * i + 1) * 64;
                
                // 预取当前处理的数据
                _mm_prefetch((char*)block1, _MM_HINT_T0);
                _mm_prefetch((char*)(block1 + 16), _MM_HINT_T0);
                _mm_prefetch((char*)block2, _MM_HINT_T0);
                _mm_prefetch((char*)(block2 + 16), _MM_HINT_T0);
                
                w1_03 = _mm_load_si128((const __m128i*)block1);
                w1_47 = _mm_load_si128((const __m128i*)(block1 + 16));
                w2_03 = _mm_load_si128((const __m128i*)block2);
                w2_47 = _mm_load_si128((const __m128i*)(block2 + 16));
                w2_74 = _mm_shuffle_epi32(w2_47, _MM_SHUFFLE(0, 1, 2, 3)); 
                w2_30 = _mm_shuffle_epi32(w2_03, _MM_SHUFFLE(0, 1, 2, 3)); 
                st0_3_first = _mm_xor_si128(w1_03, w2_74);
                st4_7_first = _mm_xor_si128(w1_47, w2_30);
                st8_11_first = _mm_shuffle_epi32(st4_7_first, _MM_SHUFFLE(0, 1, 2, 3));
                st12_15_first = _mm_shuffle_epi32(st0_3_first, _MM_SHUFFLE(0, 1, 2, 3));
                
                block1 = prev_buf + (2 * (i+1)) * 64;
                block2 = prev_buf + (2 * (i+1) + 1) * 64;
                
                // 预取第二组数据
                _mm_prefetch((char*)block1, _MM_HINT_T0);
                _mm_prefetch((char*)(block1 + 16), _MM_HINT_T0);
                _mm_prefetch((char*)block2, _MM_HINT_T0);
                _mm_prefetch((char*)(block2 + 16), _MM_HINT_T0);
                
                w1_03 = _mm_load_si128((const __m128i*)block1);
                w1_47 = _mm_load_si128((const __m128i*)(block1 + 16));
                w2_03 = _mm_load_si128((const __m128i*)block2);
                w2_47 = _mm_load_si128((const __m128i*)(block2 + 16));
                w2_74 = _mm_shuffle_epi32(w2_47, _MM_SHUFFLE(0, 1, 2, 3)); 
                w2_30 = _mm_shuffle_epi32(w2_03, _MM_SHUFFLE(0, 1, 2, 3)); 
                st0_3_second = _mm_xor_si128(w1_03, w2_74);
                st4_7_second = _mm_xor_si128(w1_47, w2_30);
                st8_11_second = _mm_shuffle_epi32(st4_7_second, _MM_SHUFFLE(0, 1, 2, 3));
                st12_15_second = _mm_shuffle_epi32(st0_3_second, _MM_SHUFFLE(0, 1, 2, 3));

                block1 = prev_buf + (2 * (i+2)) * 64;
                block2 = prev_buf + (2 * (i+2) + 1) * 64;
                
                // 预取第三组数据
                _mm_prefetch((char*)block1, _MM_HINT_T0);
                _mm_prefetch((char*)(block1 + 16), _MM_HINT_T0);
                _mm_prefetch((char*)block2, _MM_HINT_T0);
                _mm_prefetch((char*)(block2 + 16), _MM_HINT_T0);
                
                w1_03 = _mm_load_si128((const __m128i*)block1);
                w1_47 = _mm_load_si128((const __m128i*)(block1 + 16));
                w2_03 = _mm_load_si128((const __m128i*)block2);
                w2_47 = _mm_load_si128((const __m128i*)(block2 + 16));
                w2_74 = _mm_shuffle_epi32(w2_47, _MM_SHUFFLE(0, 1, 2, 3)); 
                w2_30 = _mm_shuffle_epi32(w2_03, _MM_SHUFFLE(0, 1, 2, 3)); 
                st0_3_third = _mm_xor_si128(w1_03, w2_74);
                st4_7_third = _mm_xor_si128(w1_47, w2_30);
                st8_11_third = _mm_shuffle_epi32(st4_7_third, _MM_SHUFFLE(0, 1, 2, 3));
                st12_15_third = _mm_shuffle_epi32(st0_3_third, _MM_SHUFFLE(0, 1, 2, 3));

                block1 = prev_buf + (2 * (i+3)) * 64;
                block2 = prev_buf + (2 * (i+3) + 1) * 64;
                
                // 预取第四组数据
                _mm_prefetch((char*)block1, _MM_HINT_T0);
                _mm_prefetch((char*)(block1 + 16), _MM_HINT_T0);
                _mm_prefetch((char*)block2, _MM_HINT_T0);
                _mm_prefetch((char*)(block2 + 16), _MM_HINT_T0);
                
                w1_03 = _mm_load_si128((const __m128i*)block1);
                w1_47 = _mm_load_si128((const __m128i*)(block1 + 16));
                w2_03 = _mm_load_si128((const __m128i*)block2);
                w2_47 = _mm_load_si128((const __m128i*)(block2 + 16));
                w2_74 = _mm_shuffle_epi32(w2_47, _MM_SHUFFLE(0, 1, 2, 3)); 
                w2_30 = _mm_shuffle_epi32(w2_03, _MM_SHUFFLE(0, 1, 2, 3)); 
                st0_3_fourth = _mm_xor_si128(w1_03, w2_74);
                st4_7_fourth = _mm_xor_si128(w1_47, w2_30);
                st8_11_fourth = _mm_shuffle_epi32(st4_7_fourth, _MM_SHUFFLE(0, 1, 2, 3));
                st12_15_fourth = _mm_shuffle_epi32(st0_3_fourth, _MM_SHUFFLE(0, 1, 2, 3));
                
                st0_3_quad = _mm512_inserti32x4(st0_3_quad, st0_3_first, 0);  // 插入到位置0 (bits 0-127)
                st0_3_quad = _mm512_inserti32x4(st0_3_quad, st0_3_second, 1);  // 插入到位置1 (bits 128-255)
                st0_3_quad = _mm512_inserti32x4(st0_3_quad, st0_3_third, 2);  // 插入到位置2 (bits 256-383)
                st0_3_quad = _mm512_inserti32x4(st0_3_quad, st0_3_fourth, 3);  // 插入到位置3 (bits 384-511)
                // st0_3_quad = _mm512_set_m128i(st0_3_second, st0_3_first);
                st4_7_quad = _mm512_inserti32x4(st4_7_quad, st4_7_first, 0);  // 插入到位置0 (bits 0-127)
                st4_7_quad = _mm512_inserti32x4(st4_7_quad, st4_7_second, 1);  // 插入到位置1 (bits 128-255)
                st4_7_quad = _mm512_inserti32x4(st4_7_quad, st4_7_third, 2);  // 插入到位置2 (bits 256-383)
                st4_7_quad = _mm512_inserti32x4(st4_7_quad, st4_7_fourth, 3);  // 插入到位置3 (bits 384-511)
                // st4_7_quad = _mm512_set_m128i(st4_7_second, st4_7_first);
                st8_11_quad = _mm512_inserti32x4(st8_11_quad, st8_11_first, 0);  // 插入到位置0 (bits 0-127)
                st8_11_quad = _mm512_inserti32x4(st8_11_quad, st8_11_second, 1);  // 插入到位置1 (bits 128-255)
                st8_11_quad = _mm512_inserti32x4(st8_11_quad, st8_11_third, 2);  // 插入到位置2 (bits 256-383)
                st8_11_quad = _mm512_inserti32x4(st8_11_quad, st8_11_fourth, 3);  // 插入到位置3 (bits 384-511
                st12_15_quad = _mm512_inserti32x4(st12_15_quad, st12_15_first, 0);  // 插入到位置0 (bits 0-127)
                st12_15_quad = _mm512_inserti32x4(st12_15_quad, st12_15_second, 1);  // 插入到位置1 (bits 128-255)
                st12_15_quad = _mm512_inserti32x4(st12_15_quad, st12_15_third, 2);  // 插入到位置2 (bits 256-383)
                st12_15_quad = _mm512_inserti32x4(st12_15_quad, st12_15_fourth, 3);  // 插入到位置3 (bits 384-511)
                // st4_7_quad = _mm512_set_m128i(st4_7_second, st4_7_first);
                // st8_11_quad = _mm512_set_m128i(st8_11_second, st8_11_first);
                // st12_15_quad = _mm512_set_m128i(st12_15_second, st12_15_first);
                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st4_7_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 7);
                    st8_11_quad = _mm512_add_epi32(st8_11_quad, st12_15_quad);
                    st8_11_quad = ROTL32_SIMD(st8_11_quad, 7);

                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st8_11_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 9);
                    st4_7_quad = _mm512_add_epi32(st4_7_quad, st12_15_quad);
                    st4_7_quad = ROTL32_SIMD(st4_7_quad, 9);
                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st4_7_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 7);
                    st8_11_quad = _mm512_add_epi32(st8_11_quad, st12_15_quad);
                    st8_11_quad = ROTL32_SIMD(st8_11_quad, 7);

                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st8_11_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 9);
                    st4_7_quad = _mm512_add_epi32(st4_7_quad, st12_15_quad);
                    st4_7_quad = ROTL32_SIMD(st4_7_quad, 9);
                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st4_7_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 7);
                    st8_11_quad = _mm512_add_epi32(st8_11_quad, st12_15_quad);
                    st8_11_quad = ROTL32_SIMD(st8_11_quad, 7);

                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st8_11_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 9);
                    st4_7_quad = _mm512_add_epi32(st4_7_quad, st12_15_quad);
                    st4_7_quad = ROTL32_SIMD(st4_7_quad, 9);
                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st4_7_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 7);
                    st8_11_quad = _mm512_add_epi32(st8_11_quad, st12_15_quad);
                    st8_11_quad = ROTL32_SIMD(st8_11_quad, 7);

                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st8_11_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 9);
                    st4_7_quad = _mm512_add_epi32(st4_7_quad, st12_15_quad);
                    st4_7_quad = ROTL32_SIMD(st4_7_quad, 9);
                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st4_7_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 7);
                    st8_11_quad = _mm512_add_epi32(st8_11_quad, st12_15_quad);
                    st8_11_quad = ROTL32_SIMD(st8_11_quad, 7);

                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st8_11_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 9);
                    st4_7_quad = _mm512_add_epi32(st4_7_quad, st12_15_quad);
                    st4_7_quad = ROTL32_SIMD(st4_7_quad, 9);
                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st4_7_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 7);
                    st8_11_quad = _mm512_add_epi32(st8_11_quad, st12_15_quad);
                    st8_11_quad = ROTL32_SIMD(st8_11_quad, 7);

                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st8_11_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 9);
                    st4_7_quad = _mm512_add_epi32(st4_7_quad, st12_15_quad);
                    st4_7_quad = ROTL32_SIMD(st4_7_quad, 9);
                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st4_7_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 7);
                    st8_11_quad = _mm512_add_epi32(st8_11_quad, st12_15_quad);
                    st8_11_quad = ROTL32_SIMD(st8_11_quad, 7);

                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st8_11_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 9);
                    st4_7_quad = _mm512_add_epi32(st4_7_quad, st12_15_quad);
                    st4_7_quad = ROTL32_SIMD(st4_7_quad, 9);
                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st4_7_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 7);
                    st8_11_quad = _mm512_add_epi32(st8_11_quad, st12_15_quad);
                    st8_11_quad = ROTL32_SIMD(st8_11_quad, 7);

                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st8_11_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 9);
                    st4_7_quad = _mm512_add_epi32(st4_7_quad, st12_15_quad);
                    st4_7_quad = ROTL32_SIMD(st4_7_quad, 9);
                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st4_7_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 7);
                    st8_11_quad = _mm512_add_epi32(st8_11_quad, st12_15_quad);
                    st8_11_quad = ROTL32_SIMD(st8_11_quad, 7);

                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st8_11_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 9);
                    st4_7_quad = _mm512_add_epi32(st4_7_quad, st12_15_quad);
                    st4_7_quad = ROTL32_SIMD(st4_7_quad, 9);
                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st4_7_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 7);
                    st8_11_quad = _mm512_add_epi32(st8_11_quad, st12_15_quad);
                    st8_11_quad = ROTL32_SIMD(st8_11_quad, 7);

                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st8_11_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 9);
                    st4_7_quad = _mm512_add_epi32(st4_7_quad, st12_15_quad);
                    st4_7_quad = ROTL32_SIMD(st4_7_quad, 9);
                st0_3_quad = _mm512_add_epi32(st0_3_quad,  _mm512_shuffle_epi32(st12_15_quad, _MM_SHUFFLE(0, 1, 2, 3)));
                st4_7_quad = _mm512_add_epi32(st4_7_quad,  _mm512_shuffle_epi32(st8_11_quad, _MM_SHUFFLE(0, 1, 2, 3)));

                _mm_store_si128((__m128i*)(cur_buf + i * 64), _mm512_extracti32x4_epi32(st0_3_quad, 0));
                _mm_store_si128((__m128i*)(cur_buf + i * 64 + 16), _mm512_extracti32x4_epi32(st4_7_quad, 0));
                _mm_store_si128((__m128i*)(cur_buf + i * 64 + 32), _mm512_extracti32x4_epi32(st8_11_quad, 0));
                _mm_store_si128((__m128i*)(cur_buf + i * 64 + 48), _mm512_extracti32x4_epi32(st12_15_quad, 0));

                _mm_store_si128((__m128i*)(cur_buf + (i + 1) * 64), _mm512_extracti32x4_epi32(st0_3_quad, 1));
                _mm_store_si128((__m128i*)(cur_buf + (i + 1) * 64 + 16), _mm512_extracti32x4_epi32(st4_7_quad, 1));
                _mm_store_si128((__m128i*)(cur_buf + (i + 1) * 64 + 32), _mm512_extracti32x4_epi32(st8_11_quad, 1));
                _mm_store_si128((__m128i*)(cur_buf + (i + 1) * 64 + 48), _mm512_extracti32x4_epi32(st12_15_quad, 1));

                _mm_store_si128((__m128i*)(cur_buf + (i + 2) * 64), _mm512_extracti32x4_epi32(st0_3_quad, 2));
                _mm_store_si128((__m128i*)(cur_buf + (i + 2) * 64 + 16), _mm512_extracti32x4_epi32(st4_7_quad, 2));
                _mm_store_si128((__m128i*)(cur_buf + (i + 2) * 64 + 32), _mm512_extracti32x4_epi32(st8_11_quad, 2));
                _mm_store_si128((__m128i*)(cur_buf + (i + 2) * 64 + 48), _mm512_extracti32x4_epi32(st12_15_quad, 2));

                _mm_store_si128((__m128i*)(cur_buf + (i + 3) * 64), _mm512_extracti32x4_epi32(st0_3_quad, 3));
                _mm_store_si128((__m128i*)(cur_buf + (i + 3) * 64 + 16), _mm512_extracti32x4_epi32(st4_7_quad, 3));
                _mm_store_si128((__m128i*)(cur_buf + (i + 3) * 64 + 32), _mm512_extracti32x4_epi32(st8_11_quad, 3));
                _mm_store_si128((__m128i*)(cur_buf + (i + 3) * 64 + 48), _mm512_extracti32x4_epi32(st12_15_quad, 3));
            }
        }
        else {
            #pragma omp parallel for schedule(static)
            for (int i = 0; i < length / 64; i+=4) {
                // 预取下一轮的输入数据到L1缓存
                if (i + 4 < length / 64) {
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 4)) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 4) + 1) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 5)) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 5) + 1) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 6)) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 6) + 1) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 7)) * 64), _MM_HINT_T0);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 7) + 1) * 64), _MM_HINT_T0);
                }
                
                // 预取更远的数据到L2缓存
                if (i + 8 < length / 64) {
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 8)) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 8) + 1) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 9)) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 9) + 1) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 10)) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 10) + 1) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 11)) * 64), _MM_HINT_T1);
                    _mm_prefetch((char*)(prev_buf + (2 * (i + 11) + 1) * 64), _MM_HINT_T1);
                }
                
                // 预取输出缓冲区的位置
                _mm_prefetch((char*)(cur_buf + i * 64), _MM_HINT_T0);
                _mm_prefetch((char*)(cur_buf + (i + 1) * 64), _MM_HINT_T0);
                _mm_prefetch((char*)(cur_buf + (i + 2) * 64), _MM_HINT_T0);
                _mm_prefetch((char*)(cur_buf + (i + 3) * 64), _MM_HINT_T0);
                
                __m128i w1_03, w1_47, w2_03, w2_47,w2_74,w2_30,st0_3_first, st4_7_first, st8_11_first, st12_15_first,
                        st0_3_second, st4_7_second, st8_11_second, st12_15_second,st0_3_third, st4_7_third, st8_11_third, st12_15_third,
                        st0_3_fourth, st4_7_fourth, st8_11_fourth, st12_15_fourth;
                __m512i st0_3_quad, st4_7_quad, st8_11_quad, st12_15_quad;
                uint8_t *block1 = prev_buf + (2 * i) * 64;
                uint8_t *block2 = prev_buf + (2 * i + 1) * 64;
                
                // 预取当前处理的数据
                _mm_prefetch((char*)block1, _MM_HINT_T0);
                _mm_prefetch((char*)(block1 + 16), _MM_HINT_T0);
                _mm_prefetch((char*)block2, _MM_HINT_T0);
                _mm_prefetch((char*)(block2 + 16), _MM_HINT_T0);
                
                w1_03 = _mm_load_si128((const __m128i*)block1);
                w1_47 = _mm_load_si128((const __m128i*)(block1 + 16));
                w2_03 = _mm_load_si128((const __m128i*)block2);
                w2_47 = _mm_load_si128((const __m128i*)(block2 + 16));
                w2_74 = _mm_shuffle_epi32(w2_47, _MM_SHUFFLE(0, 1, 2, 3)); 
                w2_30 = _mm_shuffle_epi32(w2_03, _MM_SHUFFLE(0, 1, 2, 3)); 
                st0_3_first = _mm_xor_si128(w1_03, w2_74);
                st4_7_first = _mm_xor_si128(w1_47, w2_30);
                st8_11_first = _mm_shuffle_epi32(st4_7_first, _MM_SHUFFLE(0, 1, 2, 3));
                st12_15_first = _mm_shuffle_epi32(st0_3_first, _MM_SHUFFLE(0, 1, 2, 3));
                
                block1 = prev_buf + (2 * (i+1)) * 64;
                block2 = prev_buf + (2 * (i+1) + 1) * 64;
                
                // 预取第二组数据
                _mm_prefetch((char*)block1, _MM_HINT_T0);
                _mm_prefetch((char*)(block1 + 16), _MM_HINT_T0);
                _mm_prefetch((char*)block2, _MM_HINT_T0);
                _mm_prefetch((char*)(block2 + 16), _MM_HINT_T0);
                
                w1_03 = _mm_load_si128((const __m128i*)block1);
                w1_47 = _mm_load_si128((const __m128i*)(block1 + 16));
                w2_03 = _mm_load_si128((const __m128i*)block2);
                w2_47 = _mm_load_si128((const __m128i*)(block2 + 16));
                w2_74 = _mm_shuffle_epi32(w2_47, _MM_SHUFFLE(0, 1, 2, 3)); 
                w2_30 = _mm_shuffle_epi32(w2_03, _MM_SHUFFLE(0, 1, 2, 3)); 
                st0_3_second = _mm_xor_si128(w1_03, w2_74);
                st4_7_second = _mm_xor_si128(w1_47, w2_30);
                st8_11_second = _mm_shuffle_epi32(st4_7_second, _MM_SHUFFLE(0, 1, 2, 3));
                st12_15_second = _mm_shuffle_epi32(st0_3_second, _MM_SHUFFLE(0, 1, 2, 3));

                block1 = prev_buf + (2 * (i+2)) * 64;
                block2 = prev_buf + (2 * (i+2) + 1) * 64;
                
                // 预取第三组数据
                _mm_prefetch((char*)block1, _MM_HINT_T0);
                _mm_prefetch((char*)(block1 + 16), _MM_HINT_T0);
                _mm_prefetch((char*)block2, _MM_HINT_T0);
                _mm_prefetch((char*)(block2 + 16), _MM_HINT_T0);
                
                w1_03 = _mm_load_si128((const __m128i*)block1);
                w1_47 = _mm_load_si128((const __m128i*)(block1 + 16));
                w2_03 = _mm_load_si128((const __m128i*)block2);
                w2_47 = _mm_load_si128((const __m128i*)(block2 + 16));
                w2_74 = _mm_shuffle_epi32(w2_47, _MM_SHUFFLE(0, 1, 2, 3)); 
                w2_30 = _mm_shuffle_epi32(w2_03, _MM_SHUFFLE(0, 1, 2, 3)); 
                st0_3_third = _mm_xor_si128(w1_03, w2_74);
                st4_7_third = _mm_xor_si128(w1_47, w2_30);
                st8_11_third = _mm_shuffle_epi32(st4_7_third, _MM_SHUFFLE(0, 1, 2, 3));
                st12_15_third = _mm_shuffle_epi32(st0_3_third, _MM_SHUFFLE(0, 1, 2, 3));

                block1 = prev_buf + (2 * (i+3)) * 64;
                block2 = prev_buf + (2 * (i+3) + 1) * 64;
                
                // 预取第四组数据
                _mm_prefetch((char*)block1, _MM_HINT_T0);
                _mm_prefetch((char*)(block1 + 16), _MM_HINT_T0);
                _mm_prefetch((char*)block2, _MM_HINT_T0);
                _mm_prefetch((char*)(block2 + 16), _MM_HINT_T0);
                
                w1_03 = _mm_load_si128((const __m128i*)block1);
                w1_47 = _mm_load_si128((const __m128i*)(block1 + 16));
                w2_03 = _mm_load_si128((const __m128i*)block2);
                w2_47 = _mm_load_si128((const __m128i*)(block2 + 16));
                w2_74 = _mm_shuffle_epi32(w2_47, _MM_SHUFFLE(0, 1, 2, 3)); 
                w2_30 = _mm_shuffle_epi32(w2_03, _MM_SHUFFLE(0, 1, 2, 3)); 
                st0_3_fourth = _mm_xor_si128(w1_03, w2_74);
                st4_7_fourth = _mm_xor_si128(w1_47, w2_30);
                st8_11_fourth = _mm_shuffle_epi32(st4_7_fourth, _MM_SHUFFLE(0, 1, 2, 3));
                st12_15_fourth = _mm_shuffle_epi32(st0_3_fourth, _MM_SHUFFLE(0, 1, 2, 3));
                
                st0_3_quad = _mm512_inserti32x4(st0_3_quad, st0_3_first, 0);  // 插入到位置0 (bits 0-127)
                st0_3_quad = _mm512_inserti32x4(st0_3_quad, st0_3_second, 1);  // 插入到位置1 (bits 128-255)
                st0_3_quad = _mm512_inserti32x4(st0_3_quad, st0_3_third, 2);  // 插入到位置2 (bits 256-383)
                st0_3_quad = _mm512_inserti32x4(st0_3_quad, st0_3_fourth, 3);  // 插入到位置3 (bits 384-511)
                // st0_3_quad = _mm512_set_m128i(st0_3_second, st0_3_first);
                st4_7_quad = _mm512_inserti32x4(st4_7_quad, st4_7_first, 0);  // 插入到位置0 (bits 0-127)
                st4_7_quad = _mm512_inserti32x4(st4_7_quad, st4_7_second, 1);  // 插入到位置1 (bits 128-255)
                st4_7_quad = _mm512_inserti32x4(st4_7_quad, st4_7_third, 2);  // 插入到位置2 (bits 256-383)
                st4_7_quad = _mm512_inserti32x4(st4_7_quad, st4_7_fourth, 3);  // 插入到位置3 (bits 384-511)
                // st4_7_quad = _mm512_set_m128i(st4_7_second, st4_7_first);
                st8_11_quad = _mm512_inserti32x4(st8_11_quad, st8_11_first, 0);  // 插入到位置0 (bits 0-127)
                st8_11_quad = _mm512_inserti32x4(st8_11_quad, st8_11_second, 1);  // 插入到位置1 (bits 128-255)
                st8_11_quad = _mm512_inserti32x4(st8_11_quad, st8_11_third, 2);  // 插入到位置2 (bits 256-383)
                st8_11_quad = _mm512_inserti32x4(st8_11_quad, st8_11_fourth, 3);  // 插入到位置3 (bits 384-511
                st12_15_quad = _mm512_inserti32x4(st12_15_quad, st12_15_first, 0);  // 插入到位置0 (bits 0-127)
                st12_15_quad = _mm512_inserti32x4(st12_15_quad, st12_15_second, 1);  // 插入到位置1 (bits 128-255)
                st12_15_quad = _mm512_inserti32x4(st12_15_quad, st12_15_third, 2);  // 插入到位置2 (bits 256-383)
                st12_15_quad = _mm512_inserti32x4(st12_15_quad, st12_15_fourth, 3);  // 插入到位置3 (bits 384-511)
                // st4_7_quad = _mm512_set_m128i(st4_7_second, st4_7_first);
                // st8_11_quad = _mm512_set_m128i(st8_11_second, st8_11_first);
                // st12_15_quad = _mm512_set_m128i(st12_15_second, st12_15_first);
                for (int round = 0; round < 10; ++round) {
                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st4_7_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 7);
                    st8_11_quad = _mm512_add_epi32(st8_11_quad, st12_15_quad);
                    st8_11_quad = ROTL32_SIMD(st8_11_quad, 7);

                    st0_3_quad = _mm512_add_epi32(st0_3_quad, st8_11_quad);
                    st0_3_quad = ROTL32_SIMD(st0_3_quad, 9);
                    st4_7_quad = _mm512_add_epi32(st4_7_quad, st12_15_quad);
                    st4_7_quad = ROTL32_SIMD(st4_7_quad, 9);
                }
                st0_3_quad = _mm512_add_epi32(st0_3_quad,  _mm512_shuffle_epi32(st12_15_quad, _MM_SHUFFLE(0, 1, 2, 3)));
                st4_7_quad = _mm512_add_epi32(st4_7_quad,  _mm512_shuffle_epi32(st8_11_quad, _MM_SHUFFLE(0, 1, 2, 3)));

                _mm_store_si128((__m128i*)(cur_buf + i * 64), _mm512_extracti32x4_epi32(st0_3_quad, 0));
                _mm_store_si128((__m128i*)(cur_buf + i * 64 + 16), _mm512_extracti32x4_epi32(st4_7_quad, 0));
                _mm_store_si128((__m128i*)(cur_buf + i * 64 + 32), _mm512_extracti32x4_epi32(st8_11_quad, 0));
                _mm_store_si128((__m128i*)(cur_buf + i * 64 + 48), _mm512_extracti32x4_epi32(st12_15_quad, 0));

                _mm_store_si128((__m128i*)(cur_buf + (i + 1) * 64), _mm512_extracti32x4_epi32(st0_3_quad, 1));
                _mm_store_si128((__m128i*)(cur_buf + (i + 1) * 64 + 16), _mm512_extracti32x4_epi32(st4_7_quad, 1));
                _mm_store_si128((__m128i*)(cur_buf + (i + 1) * 64 + 32), _mm512_extracti32x4_epi32(st8_11_quad, 1));
                _mm_store_si128((__m128i*)(cur_buf + (i + 1) * 64 + 48), _mm512_extracti32x4_epi32(st12_15_quad, 1));

                _mm_store_si128((__m128i*)(cur_buf + (i + 2) * 64), _mm512_extracti32x4_epi32(st0_3_quad, 2));
                _mm_store_si128((__m128i*)(cur_buf + (i + 2) * 64 + 16), _mm512_extracti32x4_epi32(st4_7_quad, 2));
                _mm_store_si128((__m128i*)(cur_buf + (i + 2) * 64 + 32), _mm512_extracti32x4_epi32(st8_11_quad, 2));
                _mm_store_si128((__m128i*)(cur_buf + (i + 2) * 64 + 48), _mm512_extracti32x4_epi32(st12_15_quad, 2));

                _mm_store_si128((__m128i*)(cur_buf + (i + 3) * 64), _mm512_extracti32x4_epi32(st0_3_quad, 3));
                _mm_store_si128((__m128i*)(cur_buf + (i + 3) * 64 + 16), _mm512_extracti32x4_epi32(st4_7_quad, 3));
                _mm_store_si128((__m128i*)(cur_buf + (i + 3) * 64 + 32), _mm512_extracti32x4_epi32(st8_11_quad, 3));
                _mm_store_si128((__m128i*)(cur_buf + (i + 3) * 64 + 48), _mm512_extracti32x4_epi32(st12_15_quad, 3));
            }
        }
        length /= 2;
        uint8_t *tmp = cur_buf;
        cur_buf = prev_buf;
        prev_buf = tmp;
    }
            
    memcpy(output, prev_buf, 64);
    free(cur_buf);
    free(prev_buf);
}