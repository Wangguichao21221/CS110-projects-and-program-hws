/* This project is based on the MIPS Assembler of CS61C in UC Berkeley.
   The framework of this project is been modified to be suitable for RISC-V
   in CS110 course in ShanghaiTech University by Zhijie Yang in March 2019.
   Updated by Chibin Zhang and Zhe Ye in March 2021.
*/

#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <stdint.h>

#include "block.h"
#include "tables.h"
#include "translate_utils.h"

typedef struct {
  const char* name;
  unsigned (*transform)(Block*, char**, int);
} PseudoHandler;

/* IMPLEMENT ME - see documentation in translate.c */
unsigned transform_beqz(Block* blk, char** args, int num_args);

/* IMPLEMENT ME - see documentation in translate.c */
unsigned transform_bnez(Block* blk, char** args, int num_args);

/* IMPLEMENT ME - see documentation in translate.c */
unsigned transform_li(Block* blk, char** args, int num_args);

/* IMPLEMENT ME - see documentation in translate.c */
unsigned transform_mv(Block* blk, char** args, int num_args);

/* IMPLEMENT ME - see documentation in translate.c */
unsigned transform_j(Block* blk, char** args, int num_args);

/* IMPLEMENT ME - see documentation in translate.c */
unsigned transform_jr(Block* blk, char** args, int num_args);

/* IMPLEMENT ME - see documentation in translate.c */
unsigned transform_jal(Block* blk, char** args, int num_args);

/* IMPLEMENT ME - see documentation in translate.c */
unsigned transform_jalr(Block* blk, char** args, int num_args);

/* IMPLEMENT ME - see documentation in translate.c */
unsigned transform_lw(Block* blk, char** args, int num_args);

const PseudoHandler* find_pseudo_handler(const char* name);

unsigned write_pass_one(Block* blk, const char* name, char** args,
                        int num_args);
int is_representable_by_32_bits(long num);
/* Instruction formats */
typedef enum { R_TYPE, I_TYPE, S_TYPE, SB_TYPE, U_TYPE, UJ_TYPE } InstrType;

typedef struct {
  const char* name;     /* inst name */
  InstrType instr_type; /* inst format */
  uint8_t opcode;
  uint8_t funct3;
  uint8_t funct7;   /* funct7 or part of imm */
  ImmType imm_type; /* imm type (see translate_utils.h) */
} InstrInfo;

/* IMPLEMENT ME - see documentation in translate.c */
int write_rtype(FILE* output, const InstrInfo* info, char** args,
                size_t num_args);

/* IMPLEMENT ME - see documentation in translate.c */
int write_itype(FILE* output, const InstrInfo* info, char** args,
                size_t num_args, uint32_t addr, SymbolTable* symtbl);

/* IMPLEMENT ME - see documentation in translate.c */
int write_stype(FILE* output, const InstrInfo* info, char** args,
                size_t num_args);

/* IMPLEMENT ME - see documentation in translate.c */
int write_sbtype(FILE* output, const InstrInfo* info, char** args,
                 size_t num_args, uint32_t addr, SymbolTable* symtbl);

/* IMPLEMENT ME - see documentation in translate.c */
int write_utype(FILE* output, const InstrInfo* info, char** args,
                size_t num_args, uint32_t addr, SymbolTable* symtbl);

/* IMPLEMENT ME - see documentation in translate.c */
int write_ujtype(FILE* output, const InstrInfo* info, char** args,
                 size_t num_args, uint32_t addr, SymbolTable* symtbl);

/* IMPLEMENT ME - see documentation in translate.c */
int translate_inst(FILE* output, const char* name, char** args, size_t num_args,
                   uint32_t addr, SymbolTable* symtbl);
#endif
