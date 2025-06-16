#TODO: turn f, M, K all into functions to increase readibility!!!
# Initial buffer value
  .data
 
 # Initial buffer value
 initial_buffer:
  .word 0x67452301 # A
  .word 0xefcdab89 # B
  .word 0x98badcfe # C
  .word 0x10325476 # D

# 64-element table constructed from the sine function.
K:
  .word 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee
  .word 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501
  .word 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be
  .word 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821
  .word 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa
  .word 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8
  .word 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed
  .word 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a
  .word 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c
  .word 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70
  .word 0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05
  .word 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665
  .word 0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039
  .word 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1
  .word 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1
  .word 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391

# Per-round shift amounts
S:
  .word 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22
  .word 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20
  .word 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23
  .word 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21



  .text

  .globl md5
md5:
  #prologue save all saved registers 11
  addi sp, sp, -48
  sw s0, 44(sp)
  sw s1, 40(sp)
  sw s2, 36(sp)
  sw s3, 32(sp)
  sw s4, 28(sp)
  sw s5, 24(sp)
  sw s6, 20(sp)
  sw s7, 16(sp)
  sw s8, 12(sp)
  sw s9, 8(sp)
  sw s10, 4(sp)
  sw s11, 0(sp)
  #prologue end
  mv s2, a1
  addi s0, a1, 1       # t2 = space needed = len + 1
  addi s0, s0, 192     # save space for stack (2 blocks)
  li t0, 0xfffffff0
  and s0, s0, t0 # round to
  sub  sp, sp, s0     # allocate stack space
  mv   t0, a0          # t0 = souce string pointer
  mv   t1, sp          # t1 = target stack pointer
copy_loop:
  lb   t4, 0(t0)        # 
  sb   t4, 0(t1)        # 
  beqz t4, end_copy      # 
  addi t0, t0, 1        # 
  addi t1, t1, 1        # 
  j    copy_loop
end_copy:
  addi t4 x0 512      # t4 = 512
  addi t5 x0 8        # t5 = 8 : 8 bits in a byte
  addi t3 x0 448      # t3 = 448
  addi t3,t3,-1       # t3 = 447
  mul a1 a1 t5        # len = len * 8   : bytes to bits
  mv s1, t5           # save s1 = 8
  rem t1 a1 t4        # t1 = len % 512
  div t2 a1 t4        # t2 = len / 512
  addi t2 t2 1        # t2 += 1
  blt t3, t1, bigger448     # if 447 < t1 then target (t1>= 448)
  j ctn
bigger448:
  addi t2 t2 1        # t2 += 1 :448 also append t2 = the total blocks in number
  j ctn
ctn:
  li t6, 64
  mul s3, t2, t6      #  s3 = t2 *64  s3 is the total lenth after padding
  mv s6, t2           # save s6 = total number of blocks
  mv s5, sp           # save s5 = Pointer to the beginning of the input message in the stack.
  sub s4, s3, s2      #
  li t6, 8
  sub s4, s4, t6      # s4 -= 8
  #after above, s2 = len in bytes, s3 is the total lenth after padding, 
  #s4 :bytes need to be padded, s6 = total number of blocks
  add t0, s5, s2      # t0 = the addr of the next byte of the last byte 
  li t1, 0x80         # t1 = 10000000
  sb t1, 0(t0)        # pad 10000000 after msg
  addi t0,t0, 1       # t0 +=1
  addi s4, s4, -1     # s4 -=1
  li t1, 0x00         # t1 = 00000000
paddingZeroLoop:
  beq s4 x0 endZeroloop
  sb t1, 0(t0) 
  addi t0,t0, 1
  addi s4, s4, -1
  j paddingZeroLoop
endZeroloop:
  addi t2, x0, 8
  mul s2, s2, t2     # s2 *=8 transform to lenth in bits
  sb s2, 0(t0)
  srli s2,s2,8
  sb s2, 1(t0)
  srli s2,s2,8
  sb s2, 2(t0)
  srli s2,s2,8
  sb s2, 3(t0)
  sw x0, 4(t0)       # padding zeros
  # after padding lenth 
  mv s7, s5         
  # s7 = Pointer to the beginning of the input message in the stack (used for moving pointer),s6 = total number of blocks
  mv s5, a2                      # s5 to the addr of the output
  la t0, initial_buffer
  lw s1, 0(t0)         # s1 = A
  lw s2, 4(t0)         # s2 = B
  lw s3, 8(t0)         # s3 = C
  lw s4, 12(t0)        # s4 = D  s1 - s4 are of no use
  sw s1, 0(s5)                     # 
  sw s2, 4(s5)                     # 
  sw s3, 8(s5)                     # 
  sw s4, 12(s5)                    # 
  addi s10, x0, 16                 # s10 is the number of ops in a round
ProcessLoop:
  lw s1, 0(s5)                     # 
  lw s2, 4(s5)                     # 
  lw s3, 8(s5)                     # 
  lw s4, 12(s5)                    # 
  addi s8, x0, 0                   # initialize s8 = 0 : number of round
  addi s9, x0, 0                   # initialize s9 = 0 : number of operation from 0 to 63
  beq s6, x0, endprocessloop
FourRoundLoop:
  addi t6, x0, 0              # t6 = 0
  beq s8, t6, firstRound    
  addi t6, x0, 1              # t6 = 1
  beq s8, t6, secondRound   
  addi t6, x0, 2              # t6 = 2
  beq s8, t6, thirdRound    
  addi t6, x0, 3              # t6 = 3
  beq s8, t6, fouthRound    
  addi t6, x0, 4              # t6 = 4
  beq s8, t6, endFourroundloop  # end loop
endFourroundloop:
  lw t1, 0(s5)                  # add hashes back
  lw t2, 4(s5)
  lw t3, 8(s5)
  lw t4, 12(s5)
  add t1, t1, s1
  add t2, t2, s2
  add t3, t3, s3
  add t4, t4, s4
  sw t1, 0(s5)
  sw t2, 4(s5)
  sw t3, 8(s5)
  sw t4, 12(s5)
  addi s6, s6, -1               # update total number of blocks     
  addi s7, s7, 64               # update pointer in stack
  j ProcessLoop
endprocessloop:
  add  sp, sp, s0     # 还原栈空间
  # epilogue
  lw s11, 0(sp)
  lw s10, 4(sp)
  lw s9, 8(sp)
  lw s8, 12(sp)
  lw s7, 16(sp)
  lw s6, 20(sp)
  lw s5, 24(sp)
  lw s4, 28(sp)
  lw s3, 32(sp)
  lw s2, 36(sp)
  lw s1, 40(sp)
  lw s0, 44(sp)
  addi sp, sp, 48
  ret
# can use t5, t6
firstRound: 
  addi s11, x0, 0   # initialize s11 as the operation within a round
firstRoundLoop:               # a operation in a round
  addi t5, x0, 16
  beq s11, t5, endround
  mv t3 s2                    # B is directly fed into C'
  mv t4 s3                    # C is directly fed into D',
  mv t1 s4                    # D is directly fed into A'.
  not t5, s2        # t5 = ~B
  and t6, t5, s4    # t6 = ~B & D
  and t5, s2, s3    # t5 = B & C
  or t5, t6, t5     # f = (B&C) | (~B&D) into t5
  # computing B'
  # add f
  add t2, s1, t5    # s2' = A + f
  # add M_i
  addi t0, x0, 4    # t0 = 4 bytes in a word
  mv t5 s11         # t5 = index of operation in a block
  mul t0, t5, t0    # t0 = offset in the block
  add s7, s7, t0    # s7 to the right position
  lw  t5, 0(s7)     # load a word at msg (M_i)
  sub s7, s7, t0    # s7 back to the original position
  add t2, t2, t5    # s2' = s2' + M_i
  # add K-i
  addi t0, x0, 4    # t0 = 4 bytes in a word
  mul t0, s9, t0    # t0 = offset in K
  la t5, K          # save addr of K table into t5
  add t5, t5, t0    # t5 to the right positon in K table
  lw t5, 0(t5)      # load K_i into t5
  add t2, t2, t5    # s2' = s2' + K_i
  # rotate by s bits
  addi t0, x0, 4    # t0 = 4 bytes in a word
  mul t0, s9, t0    # t0 = offset in S
  la t5, S          # save addr of K table into t5
  add t5, t5, t0    # t5 to the right positon in S table
  lw t5, 0(t5)      # load S_i into t5, bits to rotate
  mv a0 t2          # a0 = word need to be rotate
  mv a1 t5          # a1 = bits need to rotate
  addi sp, sp, -32
  sw ra, 0(sp)
  sw t1, 4(sp)
  sw t2, 8(sp)
  sw t3, 12(sp)
  sw t4, 16(sp)
  call rotl32_safe
  lw ra, 0(sp)
  lw t1, 4(sp)
  lw t2, 8(sp)
  lw t3, 12(sp)
  lw t4, 16(sp)
  addi sp, sp, 32
  mv t2, a0         # s2' after rotate (B') 
  add t2, t2, s2    # B' += B
  addi s9, s9, 1    # update number of operation 
  addi s11, s11, 1  # update number of operation within a round
  mv s1, t1         # save back to registers
  mv s2, t2
  mv s3, t3
  mv s4, t4
  j firstRoundLoop
secondRound: 
  addi s11, x0, 0   # initialize s11 as the operation within a round
secondRoundLoop:               # a operation in a round
  addi t5, x0, 16
  beq s11, t5, endround
  mv t3 s2                    # B is directly fed into C'
  mv t4 s3                    # C is directly fed into D',
  mv t1 s4                    # D is directly fed into A'.
  not t5, s4        # t5 = ~D
  and t6, t5, s3    # t6 = C ∧ ¬D
  and t5, s2, s4    # t5 = B ∧ D
  or t5, t6, t5     # f= (B ∧ D) ∨ (C ∧ ¬D)
  # computing B'
  # add f
  add t2, s1, t5    # s2' = A + f
  # add M_i
  addi t0, x0, 4    # t0 = 4 bytes in a word
  mv t5 s11         # t5 = index of operation in a block
  addi t6, x0, 5    # t6 = 5
  mul t5, t5, t6    # 5*i
  addi t5, t5, 1    # 5*1 + 1
  addi t6, x0, 16   # t6 = 16
  rem t5, t5, t6    # i = (5*i + 1) mod 16 
  mul t0, t5, t0    # t0 = offset in the block
  add s7, s7, t0    # s7 to the right position
  lw  t5, 0(s7)     # load a word at msg (M_i)
  sub s7, s7, t0    # s7 back to the original position
  add t2, t2, t5    # s2' = s2' + M_i
  # add K-i
  addi t0, x0, 4    # t0 = 4 bytes in a word
  mul t0, s9, t0    # t0 = offset in K
  la t5, K          # save addr of K table into t5
  add t5, t5, t0    # t5 to the right positon in K table
  lw t5, 0(t5)      # load K_i into t5
  add t2, t2, t5    # s2' = s2' + K_i
  # rotate by s bits
  addi t0, x0, 4    # t0 = 4 bytes in a word
  mul t0, s9, t0    # t0 = offset in S
  la t5, S          # save addr of K table into t5
  add t5, t5, t0    # t5 to the right positon in S table
  lw t5, 0(t5)      # load S_i into t5, bits to rotate
  mv a0 t2          # a0 = word need to be rotate
  mv a1 t5          # a1 = bits need to rotate
  addi sp, sp, -32
  sw ra, 0(sp)
  sw t1, 4(sp)
  sw t2, 8(sp)
  sw t3, 12(sp)
  sw t4, 16(sp)
  call rotl32_safe
  lw ra, 0(sp)
  lw t1, 4(sp)
  lw t2, 8(sp)
  lw t3, 12(sp)
  lw t4, 16(sp)
  addi sp, sp, 32
  mv t2, a0         # s2' after rotate (B') 
  add t2, t2, s2    # B' += B
  addi s9, s9, 1    # update number of operation 
  addi s11, s11, 1  # update number of operation within a round
  mv s1, t1         # save back to registers
  mv s2, t2
  mv s3, t3
  mv s4, t4
  j secondRoundLoop
thirdRound: 
  addi s11, x0, 0   # initialize s11 as the operation within a round
thirdRoundLoop:               # a operation in a round
  addi t5, x0, 16
  beq s11, t5, endround
  mv t3 s2                    # B is directly fed into C'
  mv t4 s3                    # C is directly fed into D',
  mv t1 s4                    # D is directly fed into A'.
  xor t5, s2, s3    # t5 = B ⊕ C
  xor t5, t5, s4    # t5 = t5 ⊕ D, t5 = B ⊕ C ⊕ D
  # computing B'
  # add f
  add t2, s1, t5    # s2' = A + f
  # add M_i
  addi t0, x0, 4    # t0 = 4 bytes in a word
  mv t5 s11         # t5 = index of operation in a block
  addi t6, x0, 3    # t6 = 3
  mul t5, t5, t6    # 3*i
  addi t5, t5, 5    # 3*1 + 5
  addi t6, x0, 16   # t6 = 16
  rem t5, t5, t6    # i = (3*1 + 5) mod 16 
  mul t0, t5, t0    # t0 = offset in the block
  add s7, s7, t0    # s7 to the right position
  lw  t5, 0(s7)     # load a word at msg (M_i)
  sub s7, s7, t0    # s7 back to the original position
  add t2, t2, t5    # s2' = s2' + M_i
  # add K-i
  addi t0, x0, 4    # t0 = 4 bytes in a word
  mul t0, s9, t0    # t0 = offset in K
  la t5, K          # save addr of K table into t5
  add t5, t5, t0    # t5 to the right positon in K table
  lw t5, 0(t5)      # load K_i into t5
  add t2, t2, t5    # s2' = s2' + K_i
  # rotate by s bits
  addi t0, x0, 4    # t0 = 4 bytes in a word
  mul t0, s9, t0    # t0 = offset in S
  la t5, S          # save addr of K table into t5
  add t5, t5, t0    # t5 to the right positon in S table
  lw t5, 0(t5)      # load S_i into t5, bits to rotate
  mv a0 t2          # a0 = word need to be rotate
  mv a1 t5          # a1 = bits need to rotate
  addi sp, sp, -32
  sw ra, 0(sp)
  sw t1, 4(sp)
  sw t2, 8(sp)
  sw t3, 12(sp)
  sw t4, 16(sp)
  call rotl32_safe
  lw ra, 0(sp)
  lw t1, 4(sp)
  lw t2, 8(sp)
  lw t3, 12(sp)
  lw t4, 16(sp)
  addi sp, sp, 32
  mv t2, a0         # s2' after rotate (B') 
  add t2, t2, s2    # B' += B
  addi s9, s9, 1    # update number of operation 
  addi s11, s11, 1  # update number of operation within a round
  mv s1, t1         # save back to registers
  mv s2, t2
  mv s3, t3
  mv s4, t4
  j thirdRoundLoop
fouthRound:
  addi s11, x0, 0   # initialize s11 as the operation within a round
fouthRoundLoop:               # a operation in a round
  addi t5, x0, 16
  beq s11, t5, endround
  mv t3 s2                    # B is directly fed into C'
  mv t4 s3                    # C is directly fed into D',
  mv t1 s4                    # D is directly fed into A'.
  not t5, s4        # t5 = ~D
  or t6, t5, s2     # t6 = B ∨ ¬D
  xor t5, t6, s3    # t5 = C ⊕ (B ∨ ¬D)
  # computing B'
  # add f
  add t2, s1, t5    # s2' = A + f
  # add M_i
  addi t0, x0, 4    # t0 = 4 bytes in a word
  mv t5 s11         # t5 = index of operation in a block
  addi t6, x0, 7    # t6 = 7
  mul t5, t5, t6    # 7*i
  addi t6, x0, 16   # t6 = 16
  rem t5, t5, t6    # i = (7*i) mod 16 

  mul t0, t5, t0    # t0 = offset in the block
  add s7, s7, t0    # s7 to the right position
  lw  t5, 0(s7)     # load a word at msg (M_i)
  sub s7, s7, t0    # s7 back to the original position
  add t2, t2, t5    # s2' = s2' + M_i
  # add K-i
  addi t0, x0, 4    # t0 = 4 bytes in a word
  mul t0, s9, t0    # t0 = offset in K
  la t5, K          # save addr of K table into t5
  add t5, t5, t0    # t5 to the right positon in K table
  lw t5, 0(t5)      # load K_i into t5
  add t2, t2, t5    # s2' = s2' + K_i
  # rotate by s bits
  addi t0, x0, 4    # t0 = 4 bytes in a word
  mul t0, s9, t0    # t0 = offset in S
  la t5, S          # save addr of K table into t5
  add t5, t5, t0    # t5 to the right positon in S table
  lw t5, 0(t5)      # load S_i into t5, bits to rotate
  mv a0 t2          # a0 = word need to be rotate
  mv a1 t5          # a1 = bits need to rotate
  addi sp, sp, -32
  sw ra, 0(sp)
  sw t1, 4(sp)
  sw t2, 8(sp)
  sw t3, 12(sp)
  sw t4, 16(sp)
  call rotl32_safe
  lw ra, 0(sp)
  lw t1, 4(sp)
  lw t2, 8(sp)
  lw t3, 12(sp)
  lw t4, 16(sp)
  addi sp, sp, 32
  mv t2, a0         # s2' after rotate (B') 
  add t2, t2, s2    # B' += B
  addi s9, s9, 1    # update number of operation 
  addi s11, s11, 1  # update number of operation within a round
  mv s1, t1         # save back to registers
  mv s2, t2
  mv s3, t3
  mv s4, t4
  j fouthRoundLoop
endround:
  addi s8, s8, 1   # update round number
  j FourRoundLoop  # jump to FourRoundLoop
  
# -----------------------------------------------------------------
# Function: print_message_digest
#
# Description:
#   Prints the 16-byte MD5 message digest in a human-readable hexadecimal format.
#   Each byte of the digest is converted to its two-digit hexadecimal representation,
#   resulting in a 32-character string that represents the hash.
#
# Parameters:
#   a0 - Pointer to the MD5 digest.
# -----------------------------------------------------------------
  .globl print_message_digest
print_message_digest:
  addi sp, sp, -16
  sw   ra, 8(sp)
  sw   s0, 4(sp)
  mv   s0, a0         # s0 = Pointer to the MD5 digest
  li t0, 0            # index of the byte 
print_byte_loop:
  li t1, 16           # 16 bytes in a digest
  beq t0, t1, end_print_message_digest
  add t1 s0, t0       # t1 = address of the byte
  lb  a0, 0(t1)       # load the byte
  call print_twohex   # print the byte
  addi t0, t0, 1      # next byte
  j print_byte_loop
end_print_message_digest:
  lw   ra, 8(sp)
  lw   s0, 4(sp)
  addi sp, sp, 16
  ret
  .globl print_twohex
print_twohex:
  # Print the high nibble
  addi sp, sp, -16
  sw   t1, 8(sp)
  sw   t0, 4(sp)
  mv t1,a0
  li t0, 4
  srl a0, a0, t0
  andi a0, a0, 0xF
  li t2, 10
  blt a0, t2, LessThanTen1 # if t0 < t1 then target
  addi a0, a0, 87
  j continuePrint1
LessThanTen1:
  addi a0, a0, '0'
  j continuePrint1
continuePrint1:
  mv a1,a0
  li a0 11
  ecall
  mv a0, t1
  andi a0, a0, 0xF
  li t2, 10
  blt a0, t2, LessThanTen2 # if t0 < t1 then target
  addi a0, a0, 87
  j continuePrint2
LessThanTen2:
  addi a0, a0, '0'
  j continuePrint2
continuePrint2:
  mv a1,a0
  li a0 11
  ecall
  lw   t1, 8(sp)
  lw   t0, 4(sp)
  addi sp, sp, 16
  ret

.globl rotl32_safe
rotl32_safe:
    # ===== Prolog =====
    addi sp, sp, -32    # 
    sw   t1, 16(sp)
    sw   t0, 12(sp)
    sw   s0, 8(sp)      # 
    sw   s1, 4(sp)      # 
    # =================

    # 
    mv   s0, a0         # 
    mv   s1, a1         # 
    
    sll  t0, s0, s1     # t0 = value << s
    neg  t1, s1         # t1 = -s
    addi t1, t1, 32     # t1 = 32 - s
    srl  a0, s0, t1     # a0 = value >> (32 - s)
    or   a0, a0, t0     # 
    # ===== Epilog =====
    lw   s1, 4(sp)      # 
    lw   s0, 8(sp)      # 
    lw   t1, 16(sp)
    lw   t0, 12(sp)
    addi sp, sp, 32     # 
    # =================
    
    ret                 # 