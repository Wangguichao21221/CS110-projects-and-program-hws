/* This project is based on the MIPS Assembler of CS61C in UC Berkeley.
   The framework of this project is been modified to be suitable for RISC-V
   in CS110 course in ShanghaiTech University by Zhijie Yang in March 2019.
   Updated by Chibin Zhang and Zhe Ye in March 2021.
*/

#include "tables.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "translate_utils.h"
#include "utils.h"

/* Indicates whether unique labels are supported. */
const int SYMBOLTBL_NON_UNIQUE = 0;
const int SYMBOLTBL_UNIQUE_NAME = 1;

/* The maximum length of a line. This limit will not be exceeded.
   You should create test cases with lines shorter than MAX_LINE_LEN. */
const int MAX_LINE_LEN = 256;

/*******************************
 * Helper Functions
 *******************************/

void allocation_failed(void) {
  write_to_log("Error: allocation failed\n");
  exit(1);
}

void addr_alignment_incorrect(void) {
  write_to_log("Error: address is not a multiple of 4.\n");
}

void name_already_exists(const char* name) {
  write_to_log("Error: name '%s' already exists in table.\n", name);
}

void write_sym(FILE* output, uint32_t addr, const char* name) {
  fprintf(output, "%u\t%s\n", addr, name);
}

/*******************************
 * Symbol Table Functions
 *******************************/

/* Creates a new SymbolTable containg 0 elements and returns a pointer to that
   table. Multiple SymbolTables may exist at the same time.
   If memory allocation fails, you should call allocation_failed().
   Mode will be either SYMBOLTBL_NON_UNIQUE or SYMBOLTBL_UNIQUE_NAME. You will
   need to store this value for use during add_to_table().

  The SymbolTable basicly stores Symbols in an array. If you have no idea about
   this, you can refer to create_block().

 */
SymbolTable* create_table(int mode) {
  if (!(mode == SYMBOLTBL_NON_UNIQUE || mode == SYMBOLTBL_UNIQUE_NAME)) {
    return NULL;
  }

  /* IMPLEMENT ME */
  /* === start === */
  SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
  if (!table) {
    allocation_failed();
  }
  table->len = 0;
  table->cap = INCREMENT_OF_CAP;
  table->mode = mode;
  table->entries = (Symbol*)malloc(table->cap * sizeof(Symbol));
  if (!table->entries) {
    free(table);
    allocation_failed();
  }
  return table;
  /* === end === */
}

/* Free the given SymbolTable and all associated memory. */
void free_table(SymbolTable* table) {
  /* IMPLEMENT ME */
  /* === start === */
  if (!table) {
    return;
  }
  for (uint32_t i = 0; i < table->len; i++) {
    free(table->entries[i].name);
  }
  free(table->entries);
  free(table);
  
  /* === end === */
}

static char* strdup(const char* src) {
  if (!src) {
    return NULL;
  }
  size_t len = strlen(src);
  char* dst = (char*)malloc(len + 1);
  if (!dst) {
    allocation_failed();
  }
  strcpy(dst, src);
  return dst;
}

/* Search for a label with the given name in the table.
 * If the label is found, return a pointer to the corresponding Symbol struct.
 * If the label is not found, return NULL.
 */
static Symbol* lookup(SymbolTable* table, const char* name) {
  if (!table || !name) {
    return NULL;
  }
  /* IMPLEMENT ME */
  /* === start === */
  for (uint32_t i = 0; i < table->len; i++) {
    if (strcmp(table->entries[i].name, name) == 0) {
      return &table->entries[i];
    }
  }
  /* === end === */
  return NULL;
}

/* Adds a new symbol and its address to the SymbolTable pointed to by TABLE.
   1. ADDR is given as the byte offset from the first instruction.
   2. The SymbolTable must be able to resize itself as more elements are added.

   3. Note that NAME may point to a temporary array, so it is not safe to simply
   store the NAME pointer. You must store a copy of the given string.

   4. If ADDR is not word-aligned, you should call addr_alignment_incorrect()
   and return -1.

   5. If the table's mode is SYMTBL_UNIQUE_NAME and NAME already exists
   in the table, you should call name_already_exists() and return -1.

   6.If memory allocation fails, you should call allocation_failed().

   Otherwise, you should store the symbol name and address and return 0.
 */
int add_to_table(SymbolTable* table, const char* name, uint32_t addr) {
  /* IMPLEMENT ME */
  /* === start === */
  if (!table || !name) {
    return -1;
  }
  if (addr % 4 != 0) {
    addr_alignment_incorrect();
    return -1;
  }
  if (table->mode == SYMBOLTBL_UNIQUE_NAME) {
    if (lookup(table, name) != NULL) {
      name_already_exists(name);
      return -1;
    }
  }
  if (table->len == table->cap) {
    resize_table(table);
  }
  Symbol* entry = &table->entries[table->len];
  entry->name = strdup(name);
  entry->addr = addr;
  table->len++;
  /* === end === */
  return 0;
}

/* Returns the address (byte offset) of the given symbol. If a symbol with the
   name NAME is not present in the TABLE, return -1.

   Before implementing this function, ensure that the static function
   'lookup(SymbolTable* table, const char* name)' is completed first. */

int64_t get_addr_for_symbol(SymbolTable* table, const char* name) {
  /* IMPLEMENT ME */
  /* === start === */
  Symbol* entry = lookup(table, name);
  if (entry) {
    return entry->addr;
  }
  /* === end === */
  return -1;
}

/* Writes the SymbolTable TABLE to OUTPUT. */
void write_table(SymbolTable* table, FILE* output) {
  if (!table || !output) {
    return;
  }
  for (uint32_t i = 0; i < table->len; i++) {
    Symbol* entry = &table->entries[i];
    write_sym(output, entry->addr, entry->name);
  }
}

/* Increase the capacity of the table by INCREMENT_OF_CAP */
void resize_table(SymbolTable* table) {
  /* IMPLEMENT ME */
  /* === start === */
  Symbol* new_entries =
      realloc(table->entries, (table->len + INCREMENT_OF_CAP) * sizeof(Symbol));
  if (!new_entries) {
    allocation_failed();
  }
  table->entries = new_entries;
  table->cap = table->len + INCREMENT_OF_CAP;
  /* === end === */
}
