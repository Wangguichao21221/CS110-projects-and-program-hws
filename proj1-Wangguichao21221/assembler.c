/* This project is based on the MIPS Assembler of CS61C in UC Berkeley.
   The framework of this project is been modified to be suitable for RISC-V
   in CS110 course in ShanghaiTech University by Zhijie Yang in March 2019.
   Updated by Chibin Zhang and Zhe Ye in March 2021.
*/

#include "assembler.h"

#include <getopt.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/block.h"
#include "src/tables.h"
#include "src/translate.h"
#include "src/translate_utils.h"
#include "src/utils.h"

#define MAX_ARGS 3
#define BUF_SIZE 1024
#define MAX_PATH_LENGTH 512
const char* IGNORE_CHARS = " \f\n\r\t\v,()";

/*******************************
 * Helper Functions
 *******************************/

/* you should not be calling this function yourself. */
static void raise_label_error(uint32_t input_line, const char* label) {
  write_to_log("Error - invalid label at line %d: %s\n", input_line, label);
}

/* call this function if more than MAX_ARGS arguments are found while parsing
   arguments.

   INPUT_LINE is which line of the input file that the error occurred in. Note
   that the first line is line 1 and that empty lines are included in the count.

   EXTRA_ARG should contain the first extra argument encountered.
 */
static void raise_extra_argument_error(uint32_t input_line,
                                       const char* extra_arg) {
  write_to_log("Error - extra argument at line %d: %s\n", input_line,
               extra_arg);
}

/* You should call this function if write_pass_one() or translate_inst()
   returns -1.

   INPUT_LINE is which line of the input file that the error occurred in. Note
   that the first line is line 1 and that empty lines are included in the count.
 */
static void raise_instruction_error(uint32_t input_line, const char* name,
                                    char** args, int num_args) {
  write_to_log("Error - invalid instruction at line %d: ", input_line);
  log_inst(name, args, num_args);
}

/* Truncates the string at the first occurrence of the '#' character. */
static void skip_comments(char* str) {
  char* comment_start = strchr(str, '#');
  if (comment_start) {
    *comment_start = '\0';
  }
}

/* Reads STR and determines whether it is a label (ends in ':'), and if so,
   whether it is a valid label, and then tries to add it to the symbol table.

   INPUT_LINE is which line of the input file we are currently processing. Note
   that the first line is line 1 and that empty lines are included in this
   count.

   BYTE_OFFSET is the offset of the NEXT instruction (should it exist).

   Four scenarios can happen:
    1. STR is not a label (does not end in ':'). Returns 0.
    2. STR ends in ':', but is not a valid label. Returns -1.
    3a. STR ends in ':' and is a valid label. Addition to symbol table fails.
        Returns -1.
    3b. STR ends in ':' and is a valid label. Addition to symbol table succeeds.
        Returns 1.
 */
static int add_if_label(uint32_t input_line, char* str, uint32_t byte_offset,
                        SymbolTable* symtbl) {
  /* IMPLEMENT ME */
  /* === start === */
  if (str[strlen(str) - 1] == ':') {
    str[strlen(str) - 1] = '\0';
    if (is_valid_label(str)) {
      if (add_to_table(symtbl, str, byte_offset) == -1) {
        return -1;
      }
      return 1;
    } else {
      raise_label_error(input_line, str);
      return -1;
    }
  }
  /* === end === */
  return 0;
}

/*******************************
 * Implement the Following
 *******************************/

/* First pass of the assembler.

   This function should read each line, strip all comments, scan for labels,
   and pass instructions to write_pass_one(). The symbol table should also
   been built and written to specified file. The input file may or may not
   be valid. Here are some guidelines:

    1. Only one label may be present per line. It must be the first token
   present. Once you see a label, regardless of whether it is a valid label or
   invalid label, treat the NEXT token in the same line as the beginning
   of an instruction.
    2. If the first token is not a label, treat it as the name of an
   instruction. DO NOT try to check it is a valid instruction in this pass.
    3. Everything after the instruction name should be treated as arguments to
   that instruction. If there are more than MAX_ARGS arguments, call
   raise_extra_argument_error() and pass in the first extra argument. Do
   not write that instruction to the output file (i.e., don't call
   write_pass_one())
    4. Only one instruction should be present per line. You do not need to do
   anything extra to detect this - it should be handled by guideline 3.
    5. A line containing only a label is valid. The address of the label should
   be the byte offset of the next instruction, regardless of whether there
   is a next instruction or not.
    6. If an instruction contains an immediate, you should output it AS IS.
    7. Comments should always be skipped before any further process.
    8. Invalid labels don't affect translation of subsequent instructions in
   current phase

   Just like in pass_two(), if the function encounters an error it should NOT
   exit, but process the entire file and return -1. If no errors were
   encountered, it should return 0.
 */
int pass_one(FILE* input, Block* blk, SymbolTable* table) {
  printf("pass one start\n");
  /* A buffer for line parsing. */
  char buf[BUF_SIZE];

  /* Variables for argument parsing. */
  char* args[MAX_ARGS];
  uint32_t offset = 0;
  int error = 0;
  /* For each line, there are some hints of what you should do:
      1. Skip all comments
      (see the function 'static void skip_comments(char* str)')
      2. Use `strtok()` to read the next token
      3. Handle labels
      4. Parse the instruction
 */
  while (fgets(buf, BUF_SIZE, input)) {
    int exceedMaxArgument= 0;
    char *delim =" \t(),\n";
    int instrWritten = 0;
    /* IMPLEMENT ME */
    /* === start === */
    printf("processing: %s",buf);
    skip_comments(buf);
    char *token = strtok(buf, delim);
    printf("First token: %s\n",token);
    if (token == NULL){
      blk->line_number ++;
      printf("Empty at line %d\n", blk->line_number-1);
      continue;
    }
    int LABLE = add_if_label(blk->line_number,token,offset,table);
    printf("After checking lable\n");
    if (LABLE == -1 ) {
      error++;
      printf("Invalid lable\n");
      token = strtok(NULL, delim);
    }
    else if (LABLE == 1){
      printf("valid lable\n");
      token = strtok(NULL, delim);
    }
    else if (LABLE == 0){
      printf("Not lable\n");
    }
    int argcnt = 0;
    char *instrName = token;
    token = strtok(NULL, delim);
    printf("Before tokens\n");
    while (token != NULL)
    {
      printf("Token: %s\n",token);
      if (argcnt == MAX_ARGS){
        raise_extra_argument_error(blk->line_number, token);
        error++;
        exceedMaxArgument = 1;
        break;
      }
      args[argcnt] = token;
      argcnt++;
      token = strtok(NULL, delim);
    }
    if (instrName && exceedMaxArgument == 0){
      instrWritten = write_pass_one(blk, instrName, args, argcnt);
      if (instrWritten == 0){
        raise_instruction_error(blk->line_number, instrName, args, argcnt);
        error ++;
      };
      printf("instrWritten: %d\n", instrWritten);
    }

    blk->line_number ++;
    if (instrName && exceedMaxArgument == 0){
      offset+=4*instrWritten;
    }
    /* === end === */
    printf("finish line %d\n", blk->line_number-1);
  }
  if (error>0){
    return -1;
  }
  return 0;
}

/* Second pass of the assembler.
   If an error is reached, DO NOT EXIT the function. Keep translating the rest
   of the document, and at the end, return -1. Return 0 if no errors were
   encountered. */
int pass_two(Block* blk, SymbolTable* table, FILE* output) {
  if (output == NULL) {
    printf("wrong file opened.\n");
    return -1;
  }

  if (!table) {
    printf("wrong table opened.\n");
    return -1;
  }

  /* For each line, there are some hints of what you should do:
      1. Get instruction name.
      2. Parse instruction arguments; Extra arguments should be filtered out
     in `pass_one()`, so you don't need to worry about that here.
      3. Use `translate_inst()` to translate the instruction and write to the
       output file;
      4. Or if an error occurs, call `raise_instruction_error()` instead of
     write the instruction.
   */

  /* Variables for argument parsing. */
  int error = 0;
  /* Process each instruction */
  for (uint32_t i = 0; i < blk->len; ++i) {
    Instr* inst = &blk->entries[i];
    /* IMPLEMENT ME */
    /* === start === */
    char *instrName = inst->name;
    if (translate_inst(output, instrName, inst->args, inst->arg_num, inst->addr, table) == -1){
      printf("Pass two Failed\n");
      raise_instruction_error(inst->line_number, instrName, inst->args,inst->arg_num);
      error ++;
    }
    
    /* === end === */
  }
  if (error==0){
    return 0;

  }
  else {
    return -1;
  }
}

static void close_files(int count, ...) {
  va_list args;
  va_start(args, count);

  for (int i = 0; i < count; i++) {
    FILE* file = va_arg(args, FILE*);
    if (file != NULL) {
      fclose(file);
    }
  }

  va_end(args);
}

/* Output folder is assured to end with "/" */
void ResolvePath(const char* input_filename, const char* output_folder,
                 char* output_filename, char* log_filename, char* tbl_filename,
                 char* inst_filename) {
  /* Extract the filename from the path */
  const char* fileName = strrchr(input_filename, '/');
  if (fileName) {
    /* Skip the '/' character */
    fileName++;
  } else {
    /* No '/' detected, use the pure input_filename directly. */
    fileName = input_filename;
  }

  /* Remove the extension by locating the last '.' character */
  char base[MAX_PATH_LENGTH];
  char* dot = strrchr(fileName, '.');
  size_t base_length = dot ? (size_t)(dot - fileName) : strlen(fileName);

  strncpy(base, fileName, base_length);
  /* Null-terminate the base string */
  base[base_length] = '\0';

  /* Ensure the base does not exceed the buffer size - 4 */
  size_t folder_len = strlen(output_folder);
  /* We need space for "/" (1 char), ".inst" (5 chars) and the null terminator
   * (1 char) => 7 extra characters total.
   */
  size_t extra = 7;
  size_t available = (MAX_PATH_LENGTH > folder_len + extra)
                         ? MAX_PATH_LENGTH - folder_len - extra
                         : 0;

  /* Build the final filenames */
  snprintf(output_filename, MAX_PATH_LENGTH, "%s%.*s.out", output_folder,
           (int)available, base);
  snprintf(log_filename, MAX_PATH_LENGTH, "%s%.*s.log", output_folder,
           (int)available, base);

  if (tbl_filename && inst_filename) {
    snprintf(tbl_filename, MAX_PATH_LENGTH, "%s%.*s.tbl", output_folder,
             (int)available, base);
    snprintf(inst_filename, MAX_PATH_LENGTH, "%s%.*s.inst", output_folder,
             (int)available, base);
  }
}

/* Runs the two-pass assembler. Most of the actual work is done in pass_one()
   and pass_two().
 */
int assemble(const char* in, const char* out, int test) {
  FILE *input, *output, *tbl_file, *inst_file;
  char output_filename[MAX_PATH_LENGTH];
  char log_filename[MAX_PATH_LENGTH];
  char tbl_filename[MAX_PATH_LENGTH];
  char inst_filename[MAX_PATH_LENGTH];
  int err = 0;
  printf("calling assemble\n");
  if (test) {
    ResolvePath(in, out, output_filename, log_filename, tbl_filename,
                inst_filename);
  } else {
    ResolvePath(in, out, output_filename, log_filename, NULL, NULL);
  }
  set_log_file(log_filename);

  SymbolTable* tbl = create_table(SYMBOLTBL_UNIQUE_NAME);
  Block* blk = create_block();

  input = fopen(in, "r");
  output = fopen(output_filename, "w");

  if (input == NULL || output == NULL) {
    exit(1);
  }
  printf("before pass one\n");
  // what does pass_one return"
  if (pass_one(input, blk, tbl) != 0) {
    err = 1;
  }
  printf("before pass two\n");
  if (pass_two(blk, tbl, output) != 0) {
    err = 1;
  }
  printf("After pass one and two\n");
  if (test) {
    tbl_file = fopen(tbl_filename, "w");
    inst_file = fopen(inst_filename, "w");
    write_table(tbl, tbl_file);
    write_block(blk, inst_file);
    close_files(2, tbl_file, inst_file);
  }
  if (err) {
    write_to_log("One or more errors encountered during assembly operation.\n");
  } else {
    write_to_log("Assembly operation completed successfully!\n");
  }

  free_table(tbl);
  free_block(blk);

  close_files(2, input, output);
  return err;
}

static void print_usage_and_exit(void) {
  printf("Usage:\n");
  printf("--input_file: The input file of the assembler\n");
  printf("--output_folder: The output folder of the assembler\n");
  exit(0);
}

int main(int argc, char** argv) {
  printf("Entering main!\n");
  int err;
  enum OPTIONS {
    OPT_INPUT,
    OPT_OUTPUT,
    OPT_TEST,
  };

  static struct option long_options[] = {
      {"input_file", required_argument, NULL, OPT_INPUT},
      {"output_folder", required_argument, NULL, OPT_OUTPUT},
      {"test", no_argument, NULL, OPT_TEST},
      {0, 0, 0, 0}};

  char input[MAX_PATH_LENGTH] = {0};
  char output[MAX_PATH_LENGTH] = {0};

  int opt;
  char short_options[] = "";
  int option_index = 0;

  int test = 0;
  while ((opt = getopt_long_only(argc, argv, short_options, long_options,
                                 &option_index)) != -1) {
    switch (opt) {
      case OPT_INPUT:
        strcpy(input, optarg);
        break;
      case OPT_OUTPUT:
        // If the output folder ends with "/", use it directly.
        // Else, add "/" to the end.
        if (optarg[strlen(optarg) - 1] == '/') {
          snprintf(output, MAX_PATH_LENGTH, "%s", optarg);
        } else {
          snprintf(output, MAX_PATH_LENGTH, "%s/", optarg);
        }
        break;
      case OPT_TEST:
        test = 1;
        break;
      default:
        print_usage_and_exit();
        break;
    }
  }
  if (strlen(input) == 0 || strlen(output) == 0) {
    printf("Please provide the correct input file and output folder.\n");
    return 0;
  }
  err = assemble(input, output, test);
  return err;
}
