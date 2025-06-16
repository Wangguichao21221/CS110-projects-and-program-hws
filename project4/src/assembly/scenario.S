.extern Easystr, Mediumstr, Hardstr, Expertstr,game_startstr, difficultystr
.extern Easy, Medium ,Hard ,Expert
.section .text
.globl scenario_selection
.type scenario_selection, @function
scenario_selection:
	addi sp, sp, -48
	sw s7, 44(sp)
	sw ra, 28(sp)
	sw s0, 24(sp)
	sw s1, 20(sp)
	sw s2, 16(sp)
	sw s3, 12(sp)
	sw s4, 8(sp)
	sw s5, 4(sp)
	sw s6, 0(sp)
	li s0, 0          # option = 0
	li s1, 0x8430     # gray
	li s2, 0xFFFF     # white
	mv  s3, s2    
	mv  s4, s1
	mv  s5, s1
	mv  s6, s1
	li a0, 0
	jal LCD_Clear #   LCD_Clear(BLACK);
	li a0, 40
	li a1, 2
	la a2, Easystr
	mv a3, s3
	jal LCD_ShowString #   LCD_ShowString(40, 2, (char *)"1. Easy", colors[0]);
	li a0, 40
	li a1, 20
	la a2, Mediumstr
	mv a3, s4
	jal LCD_ShowString #   LCD_ShowString(40, 20, (char *)"2. Medium", colors[1]);
	li a0, 40
	li a1, 38
	la a2, Hardstr
	mv a3, s5
	jal LCD_ShowString #   LCD_ShowString(40, 38, (char *)"3. Hard", colors[2]);
	li a0, 40
	li a1, 56
	la a2, Expertstr
	mv a3, s6
	jal LCD_ShowString 
loop:
	li a0, 3 # JOY_UP = 3
	li a1, 250
	jal is_button_pressed #   iF (is_button_pressed(JOY_UP)) {
	beq a0, zero, continue2 #   if (is_button_pressed(JOY_UP)) {
	beq s0 , zero, gray_easy #   if (option == 0) {
	li t0, 1
	beq s0 , t0, gray_medium
	li t0, 2
	beq s0 , t0, gray_hard
	li t0, 3
	beq s0 , t0, gray_expert
	j continue1
gray_easy:
  mv s3, s1
	li a0, 40
	li a1, 2
	la a2, Easystr
	mv a3, s3
	jal LCD_ShowString #   LCD_ShowString(40, 2, (char *)"1. Easy", colors[0]);
	j continue1
gray_medium:
	mv s4, s1
	li a0, 40
	li a1, 20
	la a2, Mediumstr
	mv a3, s4
	jal LCD_ShowString 
	j continue1
gray_hard:
	mv s5, s1
	li a0, 40
	li a1, 38
	la a2, Hardstr
	mv a3, s5
	jal LCD_ShowString   
	j continue1
gray_expert:
	mv s6, s1
	li a0, 40
	li a1, 56
	la a2, Expertstr
	mv a3, s6
	jal LCD_ShowString 
	j continue1
continue1: # update option and LCD
	addi s0, s0, -1
	addi s0, s0, 4
	li t0, 4
	rem s0, s0, t0

	beq s0 , zero, white_easy #   if (option == 0) {
	li t0, 1
	beq s0 , t0, white_medium
	li t0, 2
	beq s0 , t0, white_hard
	li t0, 3
	beq s0 , t0, white_expert
	j continue2
white_easy:
  mv s3, s2
	li a0, 40
	li a1, 2
	la a2, Easystr
	mv a3, s3
	jal LCD_ShowString #   LCD_ShowString(40, 2, (char *)"1. Easy", colors[0]);
	j continue2
white_medium:
	mv s4, s2
	li a0, 40
	li a1, 20
	la a2, Mediumstr
	mv a3, s4
	jal LCD_ShowString 
	j continue2
white_hard:
	mv s5, s2
	li a0, 40
	li a1, 38
	la a2, Hardstr
	mv a3, s5
	jal LCD_ShowString   
	j continue2
white_expert:
	mv s6, s2
	li a0, 40
	li a1, 56
	la a2, Expertstr
	mv a3, s6
	jal LCD_ShowString 
continue2:
	li a0, 1 # JOY_DOWN = 1
	li a1, 250
	jal is_button_pressed #   iF (is_button_pressed(JOY_DOWN)) {
	beq a0, zero, continue4 #   if (is_button_pressed(JOY_DOWN)) {
	beq s0 , zero, gray_easy2 #   if (option == 0) {
	li t0, 1
	beq s0 , t0, gray_medium2
	li t0, 2
	beq s0 , t0, gray_hard2
	li t0, 3
	beq s0 , t0, gray_expert2
	j continue3
gray_easy2:
  mv s3, s1
	li a0, 40
	li a1, 2
	la a2, Easystr
	mv a3, s3
	jal LCD_ShowString #   LCD_ShowString(40, 2, (char *)"1. Easy", colors[0]);
	j continue3
gray_medium2:
	mv s4, s1
	li a0, 40
	li a1, 20
	la a2, Mediumstr
	mv a3, s4
	jal LCD_ShowString 
	j continue3
gray_hard2:
	mv s5, s1
	li a0, 40
	li a1, 38
	la a2, Hardstr
	mv a3, s5
	jal LCD_ShowString   
	j continue3
gray_expert2:
	mv s6, s1
	li a0, 40
	li a1, 56
	la a2, Expertstr
	mv a3, s6
	jal LCD_ShowString 
	j continue3
continue3: # update option and LCD
	addi s0, s0, 1
	li t0, 4
	rem s0, s0, t0

	beq s0 , zero, white_easy2 #   if (option == 0) {
	li t0, 1
	beq s0 , t0, white_medium2
	li t0, 2
	beq s0 , t0, white_hard2
	li t0, 3
	beq s0 , t0, white_expert2
	j continue4
white_easy2:
  mv s3, s2
	li a0, 40
	li a1, 2
	la a2, Easystr
	mv a3, s3
	jal LCD_ShowString #   LCD_ShowString(40, 2, (char *)"1. Easy", colors[0]);
	j continue4
white_medium2:
	mv s4, s2
	li a0, 40
	li a1, 20
	la a2, Mediumstr
	mv a3, s4
	jal LCD_ShowString 
	j continue4
white_hard2:
	mv s5, s2
	li a0, 40
	li a1, 38
	la a2, Hardstr
	mv a3, s5
	jal LCD_ShowString   
	j continue4
white_expert2:
	mv s6, s2
	li a0, 40
	li a1, 56
	la a2, Expertstr
	mv a3, s6
	jal LCD_ShowString 
continue4:
	li a0, 4 # JOY_CTR = 4
	li a1, 250
	jal is_button_pressed #   iF (is_button_pressed(JOY_CTR)) {
	beq a0, zero, continue5 #   if (is_button_pressed(JOY_CTR)) {
	li s7, 0 # s7 is counter
loop_flash: 
	li t0, 5
	beq s7, t0, end_flash #   for (int i = 0; i < 5; i++) { // Flash 5 times (1 second total)
	beq s0 , zero, blink_easy #   if (option == 0) {
	li t0, 1
	beq s0 , t0, blink_medium
	li t0, 2
	beq s0 , t0, blink_hard
	li t0, 3
	beq s0 , t0, blink_expert
blink_easy:
	li t0,2
	rem t1, s7, t0
	beq t1, zero, white_blink_easy
	mv s3, s1
	li a0, 40
	li a1, 2
	la a2, Easystr
	mv a3, s3
	jal LCD_ShowString #   LCD_ShowString(40, 2, (char *)"1. Easy", colors[0]);
	li a0, 200
	jal delay_1ms #   delay_1ms(200); // 200ms delay for each flash
	j continue_flash
white_blink_easy:
	mv s3, s2
	li a0, 40
	li a1, 2
	la a2, Easystr
	mv a3, s3
	jal LCD_ShowString #   LCD_ShowString(40, 2, (char *)"1. Easy", colors[0]);
	li a0, 200
	jal delay_1ms #   delay_1ms(200); // 200ms delay for each flash
	j continue_flash
blink_medium:
	li t0,2
	rem t1, s7, t0
	beq t1, zero, white_blink_medium
	mv s4, s1
	li a0, 40
	li a1, 20
	la a2, Mediumstr
	mv a3, s4
	jal LCD_ShowString 
	li a0, 200
	jal delay_1ms #   delay_1ms(200); // 200ms delay for each flash
	j continue_flash
white_blink_medium:
	mv s4, s2
	li a0, 40
	li a1, 20
	la a2, Mediumstr
	mv a3, s4
	jal LCD_ShowString #   LCD_ShowString(40, 2, (char *)"1. Easy", colors[0]);
	li a0, 200
	jal delay_1ms #   delay_1ms(200); // 200ms delay for each flash
	j continue_flash
blink_hard:
	li t0,2
	rem t1, s7, t0
	beq t1, zero, white_blink_hard
	mv s5, s1
	li a0, 40
	li a1, 38
	la a2, Hardstr
	mv a3, s5
	jal LCD_ShowString #   LCD_ShowString(40, 2, (char *)"1. Easy", colors[0]);
	li a0, 200
	jal delay_1ms #   delay_1ms(200); // 200ms delay for each flash
	j continue_flash
white_blink_hard:
	mv s5, s2
	li a0, 40
	li a1, 38
	la a2, Hardstr
	mv a3, s5
	jal LCD_ShowString #   LCD_ShowString(40, 2, (char *)"1. Easy", colors[0]);
	li a0, 200
	jal delay_1ms #   delay_1ms(200); // 200ms delay for each flash
	j continue_flash
blink_expert:
	li t0,2
	rem t1, s7, t0
	beq t1, zero, white_blink_expert
	mv s6, s1
	li a0, 40
	li a1, 56
	la a2, Expertstr
	mv a3, s6
	jal LCD_ShowString #   LCD_ShowString(40, 2, (char *)"1. Easy", colors[0]);
	li a0, 200
	jal delay_1ms #   delay_1ms(200); // 200ms delay for each flash
	j continue_flash
white_blink_expert:
	mv s6, s2
	li a0, 40
	li a1, 56
	la a2, Expertstr
	mv a3, s6
	jal LCD_ShowString #   LCD_ShowString(40, 2, (char *)"1. Easy", colors[0]);
	li a0, 200
	jal delay_1ms #   delay_1ms(200); // 200ms delay for each flash
	j continue_flash
continue_flash:
	addi s7 , s7, 1
	j loop_flash
end_flash:
	li a0, 0
	jal LCD_Clear #   LCD_Clear(BLACK);
	mv a0, s0
	lw s7, 44(sp)
	lw ra, 28(sp)
	lw s0, 24(sp)
	lw s1, 20(sp)
	lw s2, 16(sp)
	lw s3, 12(sp)
	lw s4, 8(sp)
	lw s5, 4(sp)
	lw s6, 0(sp)
	addi sp, sp, 48
	ret
continue5:
	j loop


.globl scenario_display
.type scenario_display, @function

scenario_display:
	addi sp,sp, -16
	sw ra, 12(sp)
	sw s0, 8(sp)
	mv s0, a0 # s0 = difficulty
	li a0, 30
	li a1, 5
	la a2, game_startstr
	li a3, 0xFFFF
	jal LCD_ShowString #   LCD_ShowString(40, 5, (char *)"Game Start after 2 seconds", WHITE);
	# LCD_ShowString(40, 5, (u8*)"Game Start after 2 seconds", WHITE);
	li a0, 40
	li a1, 25
	la a2, difficultystr
	li a3, 0xFFFF
	jal LCD_ShowString #   LCD_ShowString(40, 25, (char *)"Difficulty: ", WHITE);
	beq s0, zero, easyscenario
	li t0, 1
	beq s0, t0, mediumscenario
	li t0, 2
	beq s0, t0, hardscenario
	li t0, 3
	beq s0, t0, expertscenario
easyscenario:
	li a0, 40
	li a1, 45
	la a2, Easy
	li a3, 0xFFFF
	jal LCD_ShowString #   LCD_ShowString(40, 45, (char *)"Easy", WHITE);
	j end_run_game
mediumscenario:
	li a0, 40
	li a1, 45
	la a2, Medium
	li a3, 0xFFFF
	jal LCD_ShowString #   LCD_ShowString(40, 45, (char *)"Medium", WHITE);
	j end_run_game
hardscenario:
	li a0, 40
	li a1, 45
	la a2, Hard
	li a3, 0xFFFF
	jal LCD_ShowString #   LCD_ShowString(40, 45, (char *)"Hard", WHITE);
	j end_run_game
expertscenario:
	li a0, 40
	li a1, 45
	la a2, Expert
	li a3, 0xFFFF
	jal LCD_ShowString #   LCD_ShowString(40, 45, (char *)"Expert", WHITE);
end_run_game:
	li a0, 2000
	jal delay_1ms #   delay_1ms(2000); // Wait for 2 seconds
	li a0, 0
	jal LCD_Clear #   LCD_Clear(BLACK);
	lw ra, 12(sp)
	lw s0, 8(sp)
	addi sp, sp, 16
	ret
  # LCD_ShowString(40, 25, (u8*)"Difficulty: ", WHITE);
  # switch (difficulty) {
  #   case 0:
  #     LCD_ShowString(40, 45, (u8*)"Easy", WHITE);
  #     break;
  #   case 1:
  #     LCD_ShowString(40, 45, (u8*)"Medium", WHITE);
  #     break;
  #   case 2:
  #     LCD_ShowString(40, 45, (u8*)"Hard", WHITE);
  #     break;
  #   case 3:
  #     LCD_ShowString(40, 45, (u8*)"Expert", WHITE);
  #     break;
  #   default:
  #     break;
  # }
  # delay_1ms(2000); // Wait for 2 seconds
  # LCD_Clear(BLACK);