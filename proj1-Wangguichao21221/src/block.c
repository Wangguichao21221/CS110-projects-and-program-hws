/* This project is based on the MIPS Assembler of CS61C in UC Berkeley.
   The framework of this project is been modified to be suiblock for RISC-V
   in CS110 course in ShanghaiTech University by Zhijie Yang in March 2019.
   Updated by Chibin Zhang and Zhe Ye in March 2021.
*/

#include "block.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "translate_utils.h"
#include "utils.h"

const int BLOCK_NON_UNIQUE = 0;
const int BLOCK_UNIQUE_NAME = 1;

extern const int MAX_LINE_LEN;
extern const uint32_t MIN_ADDR;
extern const uint32_t MAX_ADDR;

/*******************************
 * Helper Functions
 *******************************/

/* Helper function for handling block allocation failure */
void block_allocation_failed(void) {
  write_to_log("Error: allocation failed\n");
  exit(1);
}

/* Write an instruction to the destination output file */
void block_write_inst(FILE* output, Instr* entry) {
  fprintf(output, "%s", entry->name);
  for (uint32_t i = 0; i < entry->arg_num; i++)
    fprintf(output, " %s", entry->args[i]);
  fprintf(output, "\n");
}

/*******************************
 * Instr block Functions
 *******************************/

/* Create a block with no instructions and return a pointer to it.
   If memory allocation fails, call allocation_failed().
   The line number is initialized to 1.
 */
Block* create_block(void) {
  Block* block = (Block*)malloc(sizeof(Block));
  if (!block) {
    block_allocation_failed();
  }
  block->len = 0;
  block->cap = INCREMENT_OF_CAP;
  block->entries = (Instr*)malloc(block->cap * sizeof(Instr));
  block->line_number = 1;
  block->addr = 0;
  if (!block->entries) {
    free(block);
    block_allocation_failed();
  }

  return block;
}

/* Free the given Block and all associated memory. */
void free_block(Block* block) {
  if (!block) {
    return;
  }
  for (uint32_t i = 0; i < block->len; i++) {
    free(block->entries[i].name);
    for (uint32_t j = 0; j < block->entries[i].arg_num; j++) {
      free(block->entries[i].args[j]);
    }
  }
  free(block->entries);
  free(block);
}

/* Helper function to copy a string. */
static char* strdup(const char* src) {
  if (!src) {
    return NULL;
  }
  size_t len = strlen(src);
  char* dst = (char*)malloc(len + 1);
  if (!dst) {
    block_allocation_failed();
  }
  strcpy(dst, src);
  return dst;
}

/* Add a new Instr to the Block pointed to by 'block'.
   Restore the entry's line number. */
int add_to_block(Block* block, const char* name, char** args,
                 uint32_t arg_num, int isExtra) {
  if (!block || !name || !args) {
    return -1;
  }

  if (block->len == block->cap) {
    resize_block(block);
  }
  Instr* entry = &block->entries[block->len];
  entry->line_number = block->line_number;
  entry->name = strdup(name);
  entry->arg_num = arg_num;
  entry->addr = block->addr;
  if (isExtra == 0)
    block->addr += 4;
  else if (isExtra == -1)
    block->addr += 8;
  for (uint32_t i = 0; i < arg_num; ++i) {
    entry->args[i] = strdup(args[i]);
  }
  block->len++;
  return 0;
}

/* Write the contents of the block to the output file */
void write_block(Block* block, FILE* output) {
  if (!block || !output) {
    return;
  }

  for (uint32_t i = 0; i < block->len; i++) {
    Instr* entry = &block->entries[i];
    block_write_inst(output, entry);
  }
}

/* Increase the capacity of the block by INCREMENT_OF_CAP */
void resize_block(Block* block) {
  Instr* new_entries =
      realloc(block->entries, (block->len + INCREMENT_OF_CAP) * sizeof(Instr));
  if (!new_entries) {
    block_allocation_failed();
  }
  block->entries = new_entries;
  block->cap = block->len + INCREMENT_OF_CAP;
}
