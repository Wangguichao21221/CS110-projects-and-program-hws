/* This project is based on the MIPS Assembler of CS61C in UC Berkeley.
   The framework of this project is been modified to be suitable for RISC-V
   in CS110 course in ShanghaiTech University by Zhijie Yang in March 2019.
   Updated by Chibin Zhang and Zhe Ye in March 2021.
*/

#include "translate_utils.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write_inst_string(FILE* output, const char* name, char** args,
                       int num_args) {
  int i;

  fprintf(output, "%s", name);
  for (i = 0; i < num_args; i++) {
    fprintf(output, " %s", args[i]);
  }
  fprintf(output, "\n");
}

void write_inst_hex(FILE* output, uint32_t instruction) {
  fprintf(output, "0x%08X\n", instruction);
}

int is_valid_label(const char* str) {
  int first = 1;
  if (!str) {
    return 0;
  }

  while (*str) {
    if (first) {
      if (!isalpha((int)*str) && *str != '_') {
        return 0; /* does not start with letter or underscore */
      } else {
        first = 0;
      }
    } else if (!isalnum((int)*str) && *str != '_') {
      return 0; /* subsequent characters not alphanumeric */
    }
    str++;
  }
  return first ? 0 : 1; /* empty string is invalid  */
}

/* Translate the input string into a signed number. The number is then
   checked to be within the correct range based on its type. You should
   call is_valid_imm() to check if it is valid.

   The input may be in either positive or negative, and be in either
   decimal or hexadecimal format. It is also possible that the input is not
   a valid number. Fortunately, the library function strtol() can take
   care of all that (with a little bit of work from your side of course).
   Please read the documentation for strtol() carefully. Do not use strtoul()
   or any other variants.

   You should store the result into the location that OUTPUT points to. The
   function returns 0 if the conversion proceeded without errors, or -1 if an
   error occurred.
 */
int translate_num(long int* output, const char* str, ImmType type) {
  /* IMPLEMENT ME */
  /* === start === */
  char* endptr;
  long int result;

  result = strtol(str, &endptr, 0);

  if (endptr == str || *endptr != '\0') {
    return -1;
  }

  if (!is_valid_imm(result, type)) {
    return -1; 
  }

  *output = result;
  /* === end === */
  return 0;
}

typedef struct {
  const char* name;
  uint8_t number;
} RegEntry;

static const RegEntry reg_map[] = {
    {"zero", 0}, {"ra", 1},   {"sp", 2},   {"gp", 3},   {"tp", 4},
    {"t0", 5},   {"t1", 6},   {"t2", 7},   {"s0", 8},   {"fp", 8},
    {"s1", 9},   {"a0", 10},  {"a1", 11},  {"a2", 12},  {"a3", 13},
    {"a4", 14},  {"a5", 15},  {"a6", 16},  {"a7", 17},  {"s2", 18},
    {"s3", 19},  {"s4", 20},  {"s5", 21},  {"s6", 22},  {"s7", 23},
    {"s8", 24},  {"s9", 25},  {"s10", 26}, {"s11", 27}, {"t3", 28},
    {"t4", 29},  {"t5", 30},  {"t6", 31},  {"x0", 0},   {"x1", 1},
    {"x2", 2},   {"x3", 3},   {"x4", 4},   {"x5", 5},   {"x6", 6},
    {"x7", 7},   {"x8", 8},   {"x9", 9},   {"x10", 10}, {"x11", 11},
    {"x12", 12}, {"x13", 13}, {"x14", 14}, {"x15", 15}, {"x16", 16},
    {"x17", 17}, {"x18", 18}, {"x19", 19}, {"x20", 20}, {"x21", 21},
    {"x22", 22}, {"x23", 23}, {"x24", 24}, {"x25", 25}, {"x26", 26},
    {"x27", 27}, {"x28", 28}, {"x29", 29}, {"x30", 30}, {"x31", 31}};

/* Translates the register name to the corresponding register number. Please
   see the RISC-V Green Sheet for information about register numbers.
   You may use the `reg_map` above to translate registers.

   Returns the register number of STR or -1 if the register name is invalid.
 */
int translate_reg(const char* str) {
  /* IMPLEMENT ME */
  /* === start === */
  for (size_t i = 0; i < sizeof(reg_map) / sizeof(reg_map[0]); i++) {
    const RegEntry *RE = &reg_map[i];
    if (strcmp(str, RE->name) == 0) {
      return RE->number;
    }
  }
  /* === end === */
  return -1;
}

/* Validate immediate value range for type.
   Returns 1 if within the range, 0 otherwise
*/
int is_valid_imm(long imm, ImmType type) {
  switch (type) {
    case IMM_NONE:
      return 1;
    case IMM_12_SIGNED:
      return imm >= -2048 && imm <= 2047;
    /* IMPLEMENT ME */
    /* === start === */
    case IMM_13_SIGNED:
      return imm >= -4096 && imm <= 4095;
    case IMM_5_UNSIGNED:
      return imm >= 0 && imm <= 31;
    case IMM_20_UNSIGNED:
      return imm >= 0 && imm <= 1048575;
    case IMM_21_SIGNED:
      return imm >= -1048576 && imm <= 1048575;
    /* === end === */
    default:
      return 0;
  }
  return 0;  // shouldn't reach here
}
