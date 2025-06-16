add ra sp zero
sub t0 tp gp
xor t1 t2 t3 
or t4 t5 t6
sll s0 s1 s2
srl s3 s4 s5
sra s6 s7 s8
slt s9 s10 s11
sltu a0 a1 fp

label1:
mul a1 a2 a3
mulh a4 a5 a6
div a7 x0 x1
rem x3 x4 x5
addi x6 x7 -20
xori x8 x9 10

label2:
ori  x11 x12 10
andi x13 x14 12
slli x15 x16 1
srli x17 x18 2
srai x19 x20 1
slti x21 x22 2
sltiu x23 x24 4
lb x25 3(x26)
lh x27 0(x28)
lw x29 2(x30)
lbu x31 9(x2)
lhu t1 2(t2) 
jalr x1 t2 0
ecall

sb a0 0(sp)
sh a0 2(sp)
sw a0 4(sp)

beq a0 a1 4
bne a1 a2 8
blt a2 a3 12
bge a4 a5 -20
bltu s1 s2 16
bgeu s4 s5 10

lui x30 107592
auipc a1 717430
jal x13 label2

beqz x1 -4
bnez x2 -8
li x1 30
mv s1 a1
j -20
jr ra
jal label1
jalr ra
lw s1 label1

beq a0 a1 label1
bne a1 a2 label2
blt a2 a3 label1
bge a4 a5 label2
bltu s1 s2 label1
bgeu s4 s5 label2
