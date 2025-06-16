#include "cache.h"
#include <stdio.h>
uint32_t Total_memory_accesses;
uint32_t Inst_cache_hits;
uint32_t Data_cache_hits;
uint32_t Total_cache_misses;
uint32_t Total_inst_accesses;
uint32_t Total_data_accesses;
void init_cache_system(struct cache *Cache) {

  /* Your code here. */
  Cache->num_blocks = 4;
  Cache->blocks = malloc(Cache->num_blocks * sizeof(struct cache_block));
  Cache->next_to_evict = 0;
  for (int i = 0; i < Cache->num_blocks; i++) {
    Cache->blocks[i].block_size = 16;
    Cache->blocks[i].tag = 0;
    Cache->blocks[i].valid = 0;
    Cache->blocks[i].lru = 100-i;
  }
}

void add_inst_access(uint32_t address, struct cache *Cache) {
  Total_memory_accesses++;
  /* Your code here. */
  uint32_t tag = address >> 4;
  for (int i = 0; i < Cache->num_blocks; i++) {
    if (Cache->blocks[i].valid ) {
      if (Cache->blocks[i].tag == tag) {
        update_lru(Cache, i);
        // fprintf(stderr, "Inst hit at address 0x%x\n", address);
        Inst_cache_hits++;
        return;
      }
    }
  }
  // printf("Inst miss at address 0x%x, evict %u\n", address, Cache->blocks[Cache->next_to_evict]);
  Total_cache_misses++;
  Cache->blocks[Cache->next_to_evict].tag = tag;
  Cache->blocks[Cache->next_to_evict].valid = 1;
  update_lru(Cache, Cache->next_to_evict);
}

void add_data_access(uint32_t address, struct cache *Cache) {
  Total_memory_accesses++;
  /* Your code here. */
  uint32_t tag = address >> 4;
  for (int i = 0; i < Cache->num_blocks; i++) {
    if (Cache->blocks[i].valid ) {
      if (Cache->blocks[i].tag == tag) {
        // printf("Data hit at address 0x%x\n", address);
        update_lru(Cache, i);
        Data_cache_hits++;
        return;
      }
    }
  }
  // printf("Data miss at address 0x%x, evict %u\n", address, Cache->blocks[Cache->next_to_evict]);
  Total_cache_misses++;
  Cache->blocks[Cache->next_to_evict].tag = tag;
  Cache->blocks[Cache->next_to_evict].valid = 1;
  update_lru(Cache, Cache->next_to_evict);
}

void print_cache_statistics(struct cache *Cache) {
  /* Your code here. */
  printf("Total memory accesses: %u.\n", Total_memory_accesses);
  printf("Instruction cache hit: %u.\n", Inst_cache_hits);
  printf("Data cache hit: %u.\n", Data_cache_hits);
  printf("Total cache misses: %u.\n", Total_cache_misses);
}

void update_lru(struct cache *Cache, uint32_t block_num) {
  uint32_t Lru = Cache->blocks[block_num].lru;
  for (int i = 0; i < Cache->num_blocks; i++) {
    if (Cache->blocks[i].lru < Lru) {
      Cache->blocks[i].lru++;
    }
    else if (Cache->blocks[i].lru == Lru) {
      Cache->blocks[i].lru = 0;
    }
  }
  uint32_t max_lru = 0;
  for (int i = 0; i < Cache->num_blocks; i++) {
    if (Cache->blocks[i].lru > max_lru) {
      max_lru = Cache->blocks[i].lru;
      Cache->next_to_evict = i;
    }
  }
}