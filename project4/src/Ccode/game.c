#include "Ccode/game.h"
#include <utils.h>

#include "lcd/lcd.h"
#include "utils_C.h"

#define ENEMY_PER_FRAME 60
#define ENEMY_SHOOT_FRAME 10
uint64_t frame_count, frame_start_time, frame_end_time, frame_elapsed_time;

void run_game(unsigned long difficulty) {
    int last_enemy = 2;
    int x = 80, y = 75;          
    int prev_x = x, prev_y = y;  
    uint64_t enemy_frame_counter = ENEMY_PER_FRAME; 
    uint64_t enemy_shoot_frame_counter = 0;
    const uint64_t target_frame_time_ms = 1000 / 35; 
    initialize_vectors();
    while (1) {
        frame_count++;
        enemy_frame_counter++;
        enemy_shoot_frame_counter++;
        frame_start_time = get_timer_value() / (SystemCoreClock / 4000.0);

        draw_player(x, y, prev_x, prev_y);
        prev_x = x; 
        prev_y = y;

        move_player(&x, &y); 
        move_and_draw_bullets();
        player_shoot(x, y); 
        update_fps();
        show_bullets_num();
        move_and_draw_enemy();

        if (enemy_shoot_frame_counter >= ENEMY_SHOOT_FRAME) {
            enemy_shoot(); 
            enemy_shoot_frame_counter = 0; 
        } 

        if (get_num_enemies()==0) {
            last_enemy = (last_enemy+1)%3;
            if (last_enemy == 0)
                add_enemy(1, 50, 1, 3, YELLOW, ENEMY_RANDOM, 3); 
            else if (last_enemy == 1) {
                add_enemy(80, 1, 1, 1, BROWN, ENEMY_EXPLOSION, 3); 
            } else if (last_enemy == 2) {
                add_enemy(159, 20, 1, 2, BLUE, ENEMY_LINE, 3);
            }
            enemy_frame_counter = 0; 
        } 


        if (frame_elapsed_time < target_frame_time_ms) {
            delay_1ms(target_frame_time_ms - frame_elapsed_time); //keep the max fps below 40
        }
        frame_end_time = get_timer_value() / (SystemCoreClock / 4000.0);
        frame_elapsed_time = frame_end_time - frame_start_time;
    }
}