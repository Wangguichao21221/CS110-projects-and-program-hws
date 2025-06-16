# Project 1.2 MD5 in RISC-V

# You can suppose you will get the number of bytes of input message from 
# "length_of_input_message" and input message from "message". 
# Both of them are stored in file "input.S".

# When we test your project, we will change the input files content.

# DO NOT MODIFY THIS FILE WHEN SUBMITTING
.import input.S
.import md5.S

  .data
  
# Message digest
md:
  .word 0, 0, 0, 0



  .text
  .globl main

main:
  la a0, message
  lw a1, length_of_input_message
  la a2, md
  jal ra, md5
  
  la a0, md
  jal ra, print_message_digest

  # Exit with error code 0
  add	   a1, x0, x0
  addi   a0, x0, 17
  ecall