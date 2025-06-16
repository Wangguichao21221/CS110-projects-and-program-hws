/* This project is based on the MIPS Assembler of CS61C in UC Berkeley.
   The framework of this project is been modified to be suitable for RISC-V
   in CS110 course in ShanghaiTech University by Zhijie Yang in March 2019.
   Updated by Chibin Zhang and Zhe Ye in March 2021.
*/

#include "translate.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "tables.h"
#include "translate_utils.h"


static const PseudoHandler pseudo_handlers[] = {
    {"beqz", transform_beqz}, {"bnez", transform_bnez}, {"li", transform_li},
    {"mv", transform_mv},     {"j", transform_j},       {"jr", transform_jr},
    {"jal", transform_jal},   {"jalr", transform_jalr}, {"lw", transform_lw},
};

/* 
Fields per entry:
  - const char* name;         -- instr name
  - InstrType instr_type;     -- instr format, e.g. R_type
  - uint8_t opcode;
  - uint8_t funct3;
  - uint8_t funct7;           -- funct7 or partial imm
  - ImmType imm_type;         -- imm type (see translate_utils.h)
*/
static const InstrInfo instr_table[] = {
    // R-type instructions
    {"add", R_TYPE, 0x33, 0x0, 0x00, IMM_NONE},

    {"sub", R_TYPE, 0x33, 0x0, 0x20, IMM_NONE},
    {"xor", R_TYPE, 0x33, 0x4, 0x00, IMM_NONE},
    {"or", R_TYPE, 0x33, 0x6, 0x00, IMM_NONE},
    {"and", R_TYPE, 0x33, 0x7, 0x00, IMM_NONE},
    {"sll", R_TYPE, 0x33, 0x1, 0x00, IMM_NONE},
    {"srl", R_TYPE, 0x33, 0x5, 0x00, IMM_NONE},
    {"sra", R_TYPE, 0x33, 0x5, 0x20, IMM_NONE},
    {"slt", R_TYPE, 0x33, 0x2, 0x00, IMM_NONE},
    {"sltu", R_TYPE, 0x33, 0x3, 0x00, IMM_NONE},
    {"mul", R_TYPE, 0x33, 0x0, 0x01, IMM_NONE},
    {"mulh", R_TYPE, 0x33, 0x1, 0x01, IMM_NONE},
    {"div", R_TYPE, 0x33, 0x4, 0x01, IMM_NONE},
    {"rem", R_TYPE, 0x33, 0x6, 0x01, IMM_NONE},

    // I-type instructions
    {"addi", I_TYPE, 0x13, 0x0, 0x00, IMM_12_SIGNED},
    {"xori", I_TYPE, 0x13, 0x4, 0x00, IMM_12_SIGNED},
    {"ori", I_TYPE, 0x13, 0x6, 0x00, IMM_12_SIGNED},
    {"andi", I_TYPE, 0x13, 0x7, 0x00, IMM_12_SIGNED},
    {"slli", I_TYPE, 0x13, 0x1, 0x00, IMM_5_UNSIGNED},
    {"srli", I_TYPE, 0x13, 0x5, 0x00, IMM_5_UNSIGNED},
    {"srai", I_TYPE, 0x13, 0x5, 0x20, IMM_5_UNSIGNED},
    {"slti", I_TYPE, 0x13, 0x2, 0x00, IMM_12_SIGNED},
    {"sltiu", I_TYPE, 0x13, 0x3, 0x00, IMM_12_SIGNED},
    {"lb", I_TYPE, 0x03, 0x0, 0x00, IMM_12_SIGNED},
    {"lh", I_TYPE, 0x03, 0x1, 0x00, IMM_12_SIGNED},
    {"lw", I_TYPE, 0x03, 0x2, 0x00, IMM_12_SIGNED},
    {"lbu", I_TYPE, 0x03, 0x4, 0x00, IMM_12_SIGNED},
    {"lhu", I_TYPE, 0x03, 0x5, 0x00, IMM_12_SIGNED},
    {"jalr", I_TYPE, 0x67, 0x0, 0x00, IMM_12_SIGNED},
    {"ecall", I_TYPE, 0x73, 0x0, 0x00, IMM_NONE},

    // S-type instructions
    {"sb", S_TYPE, 0x23, 0x0, 0x00, IMM_12_SIGNED},
    {"sh", S_TYPE, 0x23, 0x1, 0x00, IMM_12_SIGNED},
    {"sw", S_TYPE, 0x23, 0x2, 0x00, IMM_12_SIGNED},

    // SB-type instructions
    {"beq", SB_TYPE, 0x63, 0x0, 0x00, IMM_13_SIGNED},
    {"bne", SB_TYPE, 0x63, 0x1, 0x00, IMM_13_SIGNED},
    {"blt", SB_TYPE, 0x63, 0x4, 0x00, IMM_13_SIGNED},
    {"bge", SB_TYPE, 0x63, 0x5, 0x00, IMM_13_SIGNED},
    {"bltu", SB_TYPE, 0x63, 0x6, 0x00, IMM_13_SIGNED},
    {"bgeu", SB_TYPE, 0x63, 0x7, 0x00, IMM_13_SIGNED},

    // U-type instructions
    {"lui", U_TYPE, 0x37, 0x0, 0x00, IMM_20_UNSIGNED},
    {"auipc", U_TYPE, 0x17, 0x0, 0x00, IMM_20_UNSIGNED},

    // UJ-type instructions
    {"jal", UJ_TYPE, 0x6f, 0x0, 0x00, IMM_21_SIGNED},
};

unsigned transform_beqz(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 2) {
    printf("Error: Wrong number of arguments for beqz\n");
    return 0;
  }
  char *instrName = "beq";
  char *newArgs[3] = {args[0], "x0", args[1]};
  return (add_to_block(blk, instrName, newArgs, 3, 0) == 0);
  /* === end === */
}

unsigned transform_bnez(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 2) {
    printf("Error: Wrong number of arguments for bnez\n");
    return 0;
  }
  char *instrName = "bne";
  char *newArgs[3] = {args[0], "x0", args[1]};
  return (add_to_block(blk, instrName, newArgs, 3,0) == 0);
  /* === end === */
  return 0;
}
int is_representable_by_32_bits(long num) {
  if (num >= INT32_MIN && num <= INT32_MAX) {
      return 1;
  }
  if (num >= 0 && num <= UINT32_MAX) {
      return 1;
  }
  return 0;
}
/* Hint:
  - make sure that the number is representable by 32 bits. (Hint: the number
      can be both signed or unsigned).
  - if the immediate can fit in the imm field of an addiu instruction, then
      expand li into a single addi instruction. Otherwise, expand it into
      a lui-addi pair.

  venus has slightly different translation rules for li, and it allows numbers
  larger than the largest 32 bit number to be loaded with li. You should follow
  the above rules if venus behaves differently.
*/
unsigned transform_li(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 2) {
    printf("Error: Wrong number of arguments for li\n");
    return 0;
  }
  long imm;
  translate_num(&imm, args[1], IMM_NONE);
  printf("imm: %ld\n", imm);
  if (is_valid_imm(imm, IMM_12_SIGNED)) {
    char imm_str[20];
    sprintf(imm_str, "%ld", imm);
    char *instrName = "addi";
    char *newArgs[3] = {args[0], "x0", args[1]};
    return (add_to_block(blk, instrName, newArgs, 3, 0) == 0);
  } else if (is_representable_by_32_bits(imm)) {
    long int upper_imm = (imm + 0x800) >> 12; 
    long int lower_imm = imm - (upper_imm << 12); 

    upper_imm = (upper_imm & 0xFFFFF); 

    char upper_imm_str[20];
    char lower_imm_str[20];
    sprintf(upper_imm_str, "%ld", upper_imm);
    sprintf(lower_imm_str, "%ld", lower_imm);

    char *instrName1 = "lui";
    char *instrName2 = "addi";
    char *newArgs1[2] = {args[0], upper_imm_str};
    char *newArgs2[3] = {args[0], args[0], lower_imm_str};
    printf("upper_imm: %ld, lower_imm: %ld\n", upper_imm, lower_imm);
    unsigned count = 0;
    count += add_to_block(blk, instrName1, newArgs1, 2, 1) == 0;
    count += add_to_block(blk, instrName2, newArgs2, 3, -1) == 0;
    return count;
  }
  printf("Failed li\n");
  /* === end === */
  return 0;
}

/* Hint:
  - your expansion should use the fewest number of instructions possible.
 */
unsigned transform_mv(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 2) {
    printf("Error: Wrong number of arguments for mv\n");
    return 0;
  }
  char *instrName = "addi";
  char *newArgs[3] = {args[0], args[1], "0"};
  if (add_to_block(blk, instrName, newArgs, 3, 0) == 0){
    return 1;
  }
  /* === end === */
  return 0;
}

unsigned transform_j(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 1) {
    printf("Error: Wrong number of arguments for j\n");
    return 0;
  }
  char *instrName = "jal";
  char *newArgs[2] = {"x0",args[0]};
  return (add_to_block(blk, instrName, newArgs, 2,0) == 0);
  /* === end === */
  return 0;
}

unsigned transform_jr(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 1) {
    printf("Error: Wrong number of arguments for jr\n");
    return 0;
  }
  char *instrName = "jalr";
  char *newArgs[3] = {"x0", args[0], "0"};
  return (add_to_block(blk, instrName, newArgs, 3, 0) == 0);

  /* === end === */
  return 0;
}

/* Hint:
  - Since handler is selected by instruction name, be careful about
    pseudo/regular instruction name collisions
 */
unsigned transform_jal(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 1) {
    if (num_args != 2){
      printf("Error: Wrong number of arguments for jal\n");    
      return 0;
    }
    return add_to_block(blk, "jal", args, num_args, 0) == 0;
  }
  char *instrName = "jal";
  char *newArgs[2] = {"x1", args[0]};
  return (add_to_block(blk, instrName, newArgs, 2, 0) == 0);
  /* === end === */
  return 0;
}

/* Hint:
  - Since handler is selected by instruction name, be careful about
    pseudo/regular instruction name collisions
 */
unsigned transform_jalr(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args!=1){
    if(num_args!= 3){
      printf("Error: Wrong number of arguments for jalr\n");
      return 0;
    }
    else{
      return add_to_block(blk, "jalr", args, num_args, 0) == 0;
    }
  }
  char *instrName = "jalr";
  char *newArgs[3] = {"x1", args[0], "0"};
  return (add_to_block(blk, instrName, newArgs, 3, 0) == 0);
  /* === end === */
  return 0;
}

/* Hint:
 * - You should leave the label AS IS and resolve it in pass 2.
 */
unsigned transform_lw(Block* blk, char** args, int num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 2) {
    if (num_args != 3){
      printf("Error: Wrong number of arguments for lw\n");
      return 0;
    }
    else {
      printf("Normal lw inst\n");
      return add_to_block(blk, "lw", args, num_args, 0) == 0;
    }
  }
  char *instrName = "lw";
  char *newArgs[3] = {args[0], args[1], args[0]};
  char *instrName2 = "auipc";
  char *newArgs2[3] = {args[0], args[1]};
  int count = 0;
  count+= (add_to_block(blk, instrName2, newArgs2, 2, 1) == 0);
  count+= (add_to_block(blk, instrName, newArgs, 3,-1) == 0);
  return count;
  /* === end === */
  return 0;
}

/* Traverse pseudo_handlers table to select corresponding handler by NAME */
const PseudoHandler* find_pseudo_handler(const char* name) {
  for (size_t i = 0; i < sizeof(pseudo_handlers) / sizeof(pseudo_handlers[0]);
       i++) {
    if (strcmp(name, pseudo_handlers[i].name) == 0) {
      return &pseudo_handlers[i];
    }
  }
  return NULL;
}

/* Writes instructions during the assembler's first pass to BLK. The case
   for pseudo-instructions will be handled by handlers, but you need to
   write code to complete these TRANSFORM functions, as well as dealing
   with general instructions. Your pseudoinstruction expansions should not
   have any side effects.

   BLK is the intermediate instruction block you should write to,
   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   Error checking for regular instructions are done in pass two. However, for
   pseudoinstructions, you must make sure that ARGS contains the correct number
   of arguments. You do NOT need to check whether the registers / label are
   valid, since that will be checked in part two.

   Returns the number of instructions written (so 0 if there were any errors).
 */
unsigned write_pass_one(Block* blk, const char* name, char** args,
                        int num_args) { 
  /* Deal with pseudo-instructions */
  const PseudoHandler* handler = find_pseudo_handler(name);
  if (handler) {
    return handler->transform(blk, args, num_args);
  }
  /* What about general instructions? */
  /* IMPLEMENT ME */
  /* === start === */
  return (add_to_block(blk, name, args, num_args, 0) == 0);
  /* === end === */
  return 0;
}

/* Writes the instruction in hexadecimal format to OUTPUT during pass #2.

   NAME is the name of the instruction, ARGS is an array of the arguments, and
   NUM_ARGS specifies the number of items in ARGS.

   The symbol table (SYMTBL) is given for any symbols that need to be resolved
   at this step.

   You must perform error checking on all instructions and make sure that their
   arguments are valid. If an instruction is invalid, you should not write
   anything to OUTPUT but simply return -1. venus may be a useful resource for
   this step.

   All regular instruction information is given at `instr_table`.

   Note the use of helper functions. Consider writing your own! If the function
   definition comes afterwards, you must declare it first (see translate.h).

   Returns 0 on success and -1 on error.
 */
int translate_inst(FILE* output, const char* name, char** args, size_t num_args,
                   uint32_t addr, SymbolTable* symtbl) {
  for (size_t i = 0; i < sizeof(instr_table) / sizeof(instr_table[0]); i++) {
    const InstrInfo* info = &instr_table[i];
    if (strcmp(name, info->name) == 0) {
      switch (info->instr_type) {
        case R_TYPE:
          return write_rtype(output, info, args, num_args);
        case I_TYPE:
          return write_itype(output, info, args, num_args, addr, symtbl);
        case S_TYPE:
          return write_stype(output, info, args, num_args);
        case SB_TYPE:
          return write_sbtype(output, info, args, num_args, addr, symtbl);
        case U_TYPE:
          return write_utype(output, info, args, num_args, addr, symtbl);
        case UJ_TYPE:{
          return write_ujtype(output, info, args, num_args, addr, symtbl);

        }
      }
    }
  }
  return -1;
}

/* A helper function for writing most R-type instructions. You should use
   translate_reg() to parse registers and write_inst_hex() to write to
   OUTPUT. Both are defined in translate_utils.h.

   This function is INCOMPLETE. Complete the implementation below. You will
   find bitwise operations to be the cleanest way to complete this function.
 */
int write_rtype(FILE* output, const InstrInfo* info, char** args,
                size_t num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 3) {
    return -1; 
  }

  int rd = translate_reg(args[0]);
  int rs1 = translate_reg(args[1]);
  int rs2 = translate_reg(args[2]);

  if (rd == -1 || rs1 == -1 || rs2 == -1) {
    return -1; 
  }

  uint32_t instruction = (info->funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (info->funct3 << 12) | (rd << 7) | info->opcode;
  write_inst_hex(output, instruction);
  

  /* === end === */
  return 0;
}

/* Hint:
  - Number of ARGS and immediate range of each I_type instruction may
  vary. Refer to RISC-V green card and design proper encoding rules.
  - Some instruction(s) expanded from pseudo ones may has(have) unresolved
  label(s). You need to take that special case into consideration. Refer to
  write_sbtype for detailed relative address calculation.
 */
int write_itype(FILE* output, const InstrInfo* info, char** args,
                size_t num_args, uint32_t addr, SymbolTable* symtbl) {
  /* IMPLEMENT ME */
  /* === start === */
  if (strcmp(info->name, "ecall") == 0) {
    uint32_t instruction = (0 << 20) | (0 << 15) | (0 << 12) | (0 << 7) | info->opcode;
    write_inst_hex(output, instruction);
    return 0;
  }
  if (num_args != 3) {
    return -1; 
  }

  int rd ;
  int rs1;
  long offset;
  int32_t label_addr;
  if (strcmp(info->name, "lb") == 0 || strcmp(info->name, "lh") == 0 || strcmp(info->name, "lw") == 0|| strcmp(info->name, "lhu") == 0|| strcmp(info->name, "lbu") == 0) {
    rd = translate_reg(args[0]);
    rs1 = translate_reg(args[2]);
    label_addr = get_addr_for_symbol(symtbl, args[1]);
    if (label_addr == -1) {
      if (translate_num(&offset, args[1], info->imm_type) != 0) {
        return -1; 
      }
    } else {
      offset = (label_addr - (int32_t)addr);
      printf("Type: %s, offset: %ld, label_addr: %d, addr: %d\n", info->name, offset, label_addr, (int32_t)addr);
    }
  } else {
    rd = translate_reg(args[0]);
    rs1 = translate_reg(args[1]);
    label_addr = get_addr_for_symbol(symtbl, args[2]);
    if (label_addr == -1) {
      if (translate_num(&offset, args[2], info->imm_type) != 0) {
        return -1;
      }
    } else {
      offset = (label_addr - addr);
    }
  }
  if (rd == -1 || rs1 == -1) {
    return -1; 
  }
  if (is_valid_imm(offset, info->imm_type) == 0 && label_addr != -1) {
    long int upper_imm = (offset + 0x800) >> 12; 
    long int lower_imm = offset - (upper_imm<<12); 
    offset = lower_imm;
  }
  else if (is_valid_imm(offset, info->imm_type) == 0) {
    return -1; 
  }

  uint32_t instruction;
  if (info->funct7 != 0) {
    instruction = (info->funct7 << 25) | ((offset & 0x1f) << 20) | (rs1 << 15) | (info->funct3 << 12) | (rd << 7) | info->opcode;
  } else {
    instruction = (offset & 0xfff) << 20 | (rs1 << 15) | (info->funct3 << 12) | (rd << 7) | info->opcode;
  }
  write_inst_hex(output, instruction);
  /* === end === */
  return 0;
}

int write_stype(FILE* output, const InstrInfo* info, char** args,
                size_t num_args) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 3) {
    return -1; 
  }

  int rs1 = translate_reg(args[0]);
  int rs2 = translate_reg(args[2]);
  long imm;
  int numTrans_success = translate_num(&imm,args[1],info->imm_type); 

  if (rs1 == -1 || rs2 == -1 || numTrans_success!= 0) {
    return -1; 
  }
  long int imm1, imm2;
  imm1 = imm & 0x1f; 

  imm2 = (imm >> 5) & 0x7f; 

  uint32_t instruction = info-> opcode + (imm1 << 7) + (info->funct3 << 12) + (rs2 << 15) + (rs1 << 20) + (imm2 << 25);

  write_inst_hex(output, instruction);

  /* === end === */
  return 0;
}

/* Hint:
  - the way for branch to calculate relative address. e.g. bne
     bne rs rt label
   assume the byte_offset(addr) of label is L,
   current instruction byte_offset(addr) is A
   the relative address I for label satisfy:
     L = A + I
   so the relative addres is
     I = L - A
*/
int write_sbtype(FILE* output, const InstrInfo* info, char** args,
                 size_t num_args, uint32_t addr, SymbolTable* symtbl) {
  /* IMPLEMENT ME */
  /* === start === */
 if (num_args != 3) {
    return -1; 
  }

  int rs1 = translate_reg(args[0]);
  int rs2 = translate_reg(args[1]);

  long int offset;
  int32_t label_addr = get_addr_for_symbol(symtbl, args[2]);
  if (label_addr == -1){
    int success = translate_num(&offset ,args[2],info->imm_type);
    if (success!=0){
      return -1;
    }
  }
  else {
    offset = (label_addr - (int32_t)addr);
    printf("%ld = %d - %d\n",offset, label_addr, (int32_t)addr);
  }

  if (rs1 == -1 || rs2 == -1) {
    return -1; 
  }
  if (is_valid_imm(offset, info->imm_type) == 0) {
    return -1; 
  }
  long int imm, imm1,imm2,imm3,imm4;
  imm = offset;
  imm1 = (imm >> 1) & 0xf; 
  imm2 = (imm >> 5) & 0x3f; 
  imm3 = (imm >> 11) & 0x1; 
  imm4 = (imm >> 12) & 0x1; 
  printf("SB-Type: %s, offset: %ld\n",info->name, offset);
  /* generate instruction */
  uint32_t instruction = info->opcode + (imm3 << 7) + (imm1 << 8) + (info->funct3 << 12) + (rs1 << 15) + (rs2 << 20) + (imm2 << 25) + (imm4 << 31);

  write_inst_hex(output, instruction);
  

  /* === end === */
  return 0;
}

/* Hint:
  - Some instruction(s) expanded from pseudo ones may has(have) unresolved
  label(s). You need to take that special case into consideration. Refer to
  write_sbtype for detailed relative address calculation.
 */
int write_utype(FILE* output, const InstrInfo* info, char** args,
                size_t num_args, uint32_t addr, SymbolTable* symtbl) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 2) {
    return -1; 
  }
  printf("Write uType %s, argument %s\n",info->name, args[1]);
  int rd = translate_reg(args[0]);
  long int offset;
  int32_t label_addr = get_addr_for_symbol(symtbl, args[1]);
  if (label_addr == -1){
    int success = translate_num(&offset ,args[1],info->imm_type);
    if (success!=0){
      printf("Error: %s is not a valid immediate\n", args[1]);
      return -1;
    }
  }
  else {
    offset = (label_addr - (int32_t)addr);
    printf("UType Label, inst: %s, offset: %ld\n", info->name, offset);
  }
  if (rd == -1) {
    return -1;
  }
  if (label_addr != -1) {
    offset = (offset+0x800) >> 12;
  }
  uint32_t instruction = (offset << 12) | (rd << 7) | info->opcode;
  write_inst_hex(output, instruction);
  

  /* === end === */
  return 0;
}

/* In this project there is no need to relocate labels,
   you may think about the reasons. */
int write_ujtype(FILE* output, const InstrInfo* info, char** args,
                 size_t num_args, uint32_t addr, SymbolTable* symtbl) {
  /* IMPLEMENT ME */
  /* === start === */
  if (num_args != 2) {
    return -1; 
  }
  int rd = translate_reg(args[0]);
  long int offset;
  int32_t label_addr = get_addr_for_symbol(symtbl, args[1]);
  if (label_addr == -1){
    int success = translate_num(&offset ,args[1],info->imm_type);
    if (success!=0){
      return -1;
    }
  }
  else {
    offset = (label_addr - (int32_t)addr);
    printf("Lable at inst: %s, offset: %ld\n", info->name, offset);

  }
  if (rd == -1) {
    return -1;
  }
  if (is_valid_imm(offset, info->imm_type) == 0) {
    return -1; 
  }
  long int imm, imm1, imm2, imm3, imm4;
  imm = offset;
  imm1 = (imm >> 1) & 0x3ff;
  imm2 = (imm >> 11) & 0x1;
  imm3 = (imm >> 12) & 0xff;
  imm4 = (imm >> 20) & 0x1;
  
  uint32_t instruction = info ->opcode + (rd << 7) + (imm3 << 12) + (imm2 << 20) + (imm1 << 21) + (imm4 << 31);
  write_inst_hex(output, instruction);
  /* === end === */
  return 0;
}
