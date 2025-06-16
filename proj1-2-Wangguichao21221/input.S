
  .data

# Length of Input Message
# A 32-bit integer specifying the exact number of bytes in 'message'.
  .globl length_of_input_message
length_of_input_message:
  .word 57

# Input Message
# ASCII string to be processed by the MD5 hash function.
# Ensure the length matches the integer specified above (excluding the null terminator).
  .globl message
message:
  .asciiz "woshinidie213124fhaksjfhklashflkasjdfklasfhkjlashflasjlkg"