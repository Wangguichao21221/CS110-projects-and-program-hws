#ifndef _CS110_HW6_CACHE_H_
#define _CS110_HW6_CACHE_H_
#include <stdint.h>
#include <stdlib.h>
struct cache_block
{
  uint32_t block_size;
  uint32_t tag;
  // uint32_t *data; // 4*4 bytes 
  uint32_t valid;
  uint32_t lru;
};
struct cache
{
  uint32_t num_blocks;
  struct cache_block *blocks;
  uint32_t next_to_evict;
};
void init_cache_system(struct cache *);
void add_inst_access(uint32_t, struct cache *);
void add_data_access(uint32_t, struct cache *);
void print_cache_statistics();
void update_lru(struct cache *Cache, uint32_t block_num);
#endif
