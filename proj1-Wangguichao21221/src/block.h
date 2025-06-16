/* This project is based on the MIPS Assembler of CS61C in UC Berkeley.
   The framework of this project is been modified to be suiblock for RISC-V
   in CS110 course in ShanghaiTech University by Zhijie Yang in March 2019.
   Updated by Chibin Zhang and Zhe Ye in March 2021.
*/

#ifndef INSTR_blockS_H
#define INSTR_blockS_H

#include <stdint.h>
#include <stdio.h>

#define MAX_ARGS 3
/* increment of capacity */
#define INCREMENT_OF_CAP 32

/* The `Instr` structure represents an instruction. It stores the instruction
 * name, the number of arguments, the arguments themselves, and the line number
 * where the instruction appears.
 */
typedef struct {
  /* Instruction name */
  char* name;

  /* Number of arguments */
  uint32_t arg_num;

  /* Arguments for this instruction (e.g., register number, label, address,
   * immediate, etc.) */
  char* args[MAX_ARGS];
  int32_t addr;

  /* Line number of this instruction in the source file */
  int line_number;
} Instr;

/*
The `Block` structure is a container for multiple instructions. It maintains a
dynamic list of `Instr` entries, along with metadata such as the current length
(`len`), capacity (`cap`), block name, and the line number of the next
instruction to be processed.

**Note:** The `Block` structure has no relationship with a Basic Block in a
Control Flow Graph (CFG). Regardless of how many branches exist in the source
file, only one `Block` will be created, containing all valid instructions in the
first pass. You should not split a `Block` based on labels, as it serves merely
as a storage structure for instructions, preparing for label-address
substitution in the second pass
.
### Example Usage
Consider the following test case:
```
add a2 a0 a1
or a4 a3 t0
jalr zero t1 0
```
After processing this input, the `Block` should have:
- `len = 3` (indicating three instructions are stored)
- `line_number = 4` (since line numbers start from 1, and the next instruction
to process is at line 4)

Each line of the input corresponds to an `Instr` entry in the `entries` array.
For example, the first instruction (`add a2 a0 a1`) is stored in `entries[0]`
with the following properties:

```c
entries[0].name = "add";
entries[0].arg_num = 3;
entries[0].line_number = 1;
entries[0].args = {"a2", "a0", "a1"};
```
This ensures that each instruction is correctly parsed and stored in the
`Block`.

There are no functions to be implemented in these files, but understanding the
purpose of these two structures will help you better complete the assembler in
the future.
*/

typedef struct {
  /* Pointer to an array of instructions in the block */
  Instr* entries;

  /* Number of instructions currently stored in the block */
  uint32_t len;

  /* Total capacity of the block */
  uint32_t cap;

  /* The line number in the source file where the block is defined */
  uint32_t line_number;

  uint32_t addr;
} Block;

/* Helper function for handling block allocation failure */
void block_allocation_failed(void);

/* Create and initialize a new block */
Block* create_block();

/* Free a previously allocated block */
void free_block(Block* block);

/* Increase the capacity of the block by INCREMENT_OF_CAP */
void resize_block(Block* block);

/* Add an instruction to the given block */
int add_to_block(Block* block, const char* name, char** args, uint32_t arg_num, int isExtra);

/* Write the contents of the block to the output file */
void write_block(Block* block, FILE* output);

#endif
