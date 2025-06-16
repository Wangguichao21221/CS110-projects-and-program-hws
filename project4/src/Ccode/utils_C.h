#ifndef CS110_PROJECT4_UTILS_C_H
#define CS110_PROJECT4_UTILS_C_H
#include "lcd/lcd.h"
#include "utils.h"
#include <stdint.h>
#include <math.h>
#define MAX_POINTS 300

enum {
  BULLET_X,
  BULLET_POINT,
  BULLET_EXPLOSION,
  BULLET_LINE,
  BULLET_NORMAL_LINE,
};
typedef struct {
  float x;
  float y;
  float speed;

  uint32_t direction; // from 0 to 360
  uint32_t color;
  uint32_t active;
  uint32_t type;
  uint32_t size;
} Bullet; //32 bytes 2^5


struct vector {
  float x;
  float y;
};
struct vector difvectors[72];
static inline struct vector from_angle_to_vector(unsigned angle) {
    struct vector v;
    v.x = cos(angle / 180.0 * M_PI);
    v.y = -sin(angle / 180.0 * M_PI); // 负号是因为屏幕坐标系的 y 轴向下
    return v;
}
static inline initialize_vectors(){
    for (int i =0 ; i!=72;i++){
        struct vector v = from_angle_to_vector(i*5);
        difvectors[i].x = v.x;
        difvectors[i].y = v.y;
    }
}
uint32_t next_available = 0;
uint64_t num_bullets = 0; // 计数器
extern uint64_t frame_elapsed_time;
Bullet bullets[MAX_POINTS];  // 定义 bullets

static inline uint64_t get_num_bullets() {
    return num_bullets;
}

static inline void add_bullets(int x, int y, unsigned long direction, int speed, int color, int type, int size) {
    if (num_bullets >= MAX_POINTS) {
        return; 
    }
    if (!bullets[next_available].active) {
        int i = next_available;
        bullets[i].x = x;
        bullets[i].y = y; 
        bullets[i].direction = direction;
        bullets[i].speed = speed;
        bullets[i].color = color;
        bullets[i].active = 1; 
        bullets[i].type = type;
        bullets[i].size = size;
        num_bullets++; 
        next_available++;
        next_available %= MAX_POINTS; 
        return;
    }
    for (int i = 0; i < MAX_POINTS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = x;
            bullets[i].y = y;
            bullets[i].direction = direction;
            bullets[i].speed = speed;
            bullets[i].color = color;
            bullets[i].active = 1;
            bullets[i].type = type;
            bullets[i].size = size;
            num_bullets++;
            next_available = (i + 1) % MAX_POINTS; // 更新下一个可用的子弹索引
            return;
        }
    }
}

static inline void remove_bullets(int index) {
    if (bullets[index].active) {
        bullets[index].active = 0; // 标记子弹为非激活状态
        next_available = index; // 更新下一个可用的子弹索引
        num_bullets--; // 减少子弹计数
    }
}




static inline void move_and_draw_bullets() {
    for (int i = 0; i < MAX_POINTS; i++) {
        if (bullets[i].active) {
            int prev_x = round(bullets[i].x);
            int prev_y = round(bullets[i].y);
            struct vector v = difvectors[bullets[i].direction / 5]; // 获取方向对应的向量
            bullets[i].x += v.x * bullets[i].speed;
            bullets[i].y += v.y * bullets[i].speed;
            int new_x = round(bullets[i].x);
            int new_y = round(bullets[i].y);
            // 清除旧位置
            switch (bullets[i].type) {
                case BULLET_X:
                    LCD_DrawLine(prev_x - bullets[i].size, prev_y - bullets[i].size,
                        prev_x + bullets[i].size, prev_y + bullets[i].size, BLACK);
                    LCD_DrawLine(prev_x - bullets[i].size, prev_y + bullets[i].size,
                        prev_x + bullets[i].size, prev_y - bullets[i].size, BLACK);
                    break;
                case BULLET_POINT:
                    LCD_Fill(prev_x - bullets[i].size, prev_y - bullets[i].size,
                        prev_x + bullets[i].size, prev_y + bullets[i].size, BLACK);
                    break;
                case BULLET_EXPLOSION:
                LCD_Fill(prev_x - bullets[i].size, prev_y - bullets[i].size,
                    prev_x + bullets[i].size, prev_y + bullets[i].size, BLACK);
                    break;
                case BULLET_LINE:
                    LCD_DrawLine(prev_x, prev_y, prev_x + bullets[i].size, prev_y + bullets[i].size, BLACK);
                    break;
                case BULLET_NORMAL_LINE:
                    LCD_DrawLine(prev_x, prev_y, prev_x, prev_y + bullets[i].size, BLACK);
                    break;
            }

            switch (bullets[i].type) {
                case BULLET_X:
                    break;
                case BULLET_POINT:
                    break;
                case BULLET_EXPLOSION:
                    bullets[i].speed-= 0.1; 
                    if (bullets[i].speed <= 0) {
                        for (int j = 0; j < 18; j++) {
                            add_bullets(bullets[i].x, bullets[i].y, j * 20, 2, bullets[i].color, BULLET_POINT, 0); // explosion effect
                        }
                        remove_bullets(i); // remove the explosion bullet
                        continue; // skip the rest of the loop for explosion bullet
                    }
                    break;
                case BULLET_LINE:
                    bullets[i].direction +=4;  // rotate towards a direction
                    break;
                case BULLET_NORMAL_LINE:
                    break;
            }


            // 如果子弹位置未改变，跳过绘制
            if (prev_x == new_x && prev_y == new_y) {
                continue;
            }

            // 检查子弹是否超出边界
            if ( bullets[i].speed <= 0 ||new_x < 1 || new_x > 161 || new_y < 1 || new_y > 81) {
                remove_bullets(i); 
            } else {
                // 绘制新位置
                switch (bullets[i].type) {
                    case BULLET_X:
                    LCD_DrawLine(new_x - bullets[i].size, new_y - bullets[i].size,
                        new_x + bullets[i].size, new_y + bullets[i].size, bullets[i].color);
                    LCD_DrawLine(new_x - bullets[i].size, new_y + bullets[i].size,
                        new_x + bullets[i].size, new_y - bullets[i].size, bullets[i].color);
                    break;
                case BULLET_POINT:
                    LCD_Fill(new_x - bullets[i].size, new_y - bullets[i].size,
                        new_x + bullets[i].size, new_y + bullets[i].size, bullets[i].color);
                    break;
                case BULLET_EXPLOSION:
                    LCD_Fill(new_x - bullets[i].size, new_y - bullets[i].size,
                    new_x + bullets[i].size, new_y + bullets[i].size, bullets[i].color);
                    break;
                case BULLET_LINE:
                    LCD_DrawLine(new_x, new_y, new_x + bullets[i].size, new_y + bullets[i].size, bullets[i].color);
                    break;
                case BULLET_NORMAL_LINE:
                    LCD_DrawLine(new_x, new_y, new_x, new_y + bullets[i].size, bullets[i].color);
                    break;  
                }
            }
        }
    }
}

static inline void show_bullets_num() {
    static uint64_t max_bullets = 0;  
    static uint64_t elapsedtime = 0;
    if (max_bullets < num_bullets){
        max_bullets = num_bullets; // 更新最大子弹数
    }
    elapsedtime += frame_elapsed_time;
    if (elapsedtime >= 200) {


        LCD_ShowNum(125, 60, max_bullets, 3, WHITE);

        elapsedtime = 0;
        max_bullets = 0;
    }
}

#define MAX_ENEMIES 3
struct enemy
{
    int x;
    int y;
    int speed;
    int direction; // 0: up, 1: down, 2: left, 3: right
    int color;
    int active; // 1: active, 0: inactive
    int type;
    int size;
};
struct enemy enemys[MAX_ENEMIES];  
uint64_t num_enemies = 0; // enemy count in total
enum{
    ENEMY_RANDOM,
    ENEMY_EXPLOSION,
    ENEMY_LINE,
};
static inline void remove_enemy(int i); // 在调用之前声明
static inline void move_and_draw_enemy(){
    for (int i = 0; i!=MAX_ENEMIES;i++){
        if (enemys[i].active){
            // Clear the previous position
            LCD_DrawRectangle(enemys[i].x-enemys[i].size, enemys[i].y-enemys[i].size, enemys[i].x+enemys[i].size,enemys[i].y+enemys[i].size, BLACK);
            // Move the enemy
            switch (enemys[i].direction) {
                case 0: // up
                    enemys[i].y -= enemys[i].speed;
                    break;
                case 1: // down
                    enemys[i].y += enemys[i].speed;
                    break;
                case 2: // left
                    enemys[i].x -= enemys[i].speed;
                    break;
                case 3: // right
                    enemys[i].x += enemys[i].speed;
                    break;
            }
            // Draw the new position
            if (enemys[i].y < 1 || enemys[i].y > 81 || enemys[i].x < 1 || enemys[i].x > 161){
                remove_enemy(i);
            }
            else{
                LCD_DrawRectangle(enemys[i].x-enemys[i].size, enemys[i].y-enemys[i].size, enemys[i].x+enemys[i].size,enemys[i].y+enemys[i].size, enemys[i].color);
            }
            

        }
    }
};

static inline void add_enemy(int x, int y, int speed, int direction, int color, int type, int size){
    switch (type) // Check the type of enemy
    {
        case ENEMY_RANDOM:
        if (!enemys[0].active) { // Find an empty enemy slot
            enemys[0].x = x; // Set enemy's x coordinate
            enemys[0].y = y; // Set enemy's y coordinate
            enemys[0].speed = speed; // Set enemy's speed
            enemys[0].direction = direction; // Set enemy's direction
            enemys[0].color = color; // Set enemy's color
            enemys[0].active = 1; // Mark enemy as active
            enemys[0].type = type; // Set enemy's type
            enemys[0].size = size; // Set enemy's size
            num_enemies++; // Increase enemy count
            return; // Return after adding the enemy
        }
            break;
        case ENEMY_EXPLOSION:
        if (!enemys[1].active) { // Find an empty enemy slot
            enemys[1].x = x; // Set enemy's x coordinate
            enemys[1].y = y; // Set enemy's y coordinate
            enemys[1].speed = speed; // Set enemy's speed
            enemys[1].direction = direction; // Set enemy's direction
            enemys[1].color = color; // Set enemy's color
            enemys[1].active = 1; // Mark enemy as active
            enemys[1].type = type; // Set enemy's type
            enemys[1].size = size; // Set enemy's size
            num_enemies++; // Increase enemy count
            return; // Return after adding the enemy
        }
            break;
        case ENEMY_LINE:
        if (!enemys[2].active) { // Find an empty enemy slot
            enemys[2].x = x; // Set enemy's x coordinate
            enemys[2].y = y; // Set enemy's y coordinate
            enemys[2].speed = speed; // Set enemy's speed
            enemys[2].direction = direction; // Set enemy's direction
            enemys[2].color = color; // Set enemy's color
            enemys[2].active = 1; // Mark enemy as active
            enemys[2].type = type; // Set enemy's type
            enemys[2].size = size; // Set enemy's size
            num_enemies++; // Increase enemy count
            return; // Return after adding the enemy
        }
            break;
    }
};
static inline void remove_enemy(int i){
    if (enemys[i].active) {
        enemys[i].active = 0; // Mark enemy as inactive
        num_enemies--; // Decrease enemy count
    }
};
static inline int get_num_enemies(){
    return num_enemies;
};
static inline void enemy_shoot(){
    for (int i = 0; i != MAX_ENEMIES; i++)
        if (enemys[i].active){
            switch (enemys[i].type) {
                case ENEMY_LINE:
                    for (int j = 0;j != 18;j++){
                        add_bullets(enemys[i].x, enemys[i].y,j * 20, 3, WHITE, BULLET_LINE,4);
                    }
                    add_bullets(enemys[i].x, enemys[i].y, 270, 1, GREEN, BULLET_NORMAL_LINE,4);
                    break;
                case ENEMY_EXPLOSION:
                    add_bullets(enemys[i].x, enemys[i].y, 0, 3, RED, BULLET_EXPLOSION,2);
                    add_bullets(enemys[i].x, enemys[i].y,45, 3, RED, BULLET_EXPLOSION,2);
                    add_bullets(enemys[i].x, enemys[i].y,135, 3, RED, BULLET_EXPLOSION,2);
                    add_bullets(enemys[i].x, enemys[i].y,180, 3, RED, BULLET_EXPLOSION,2);
                    
                    break;
                case ENEMY_RANDOM:
                    for (int j = 0;j != 128;j++){
                        add_bullets(enemys[i].x, enemys[i].y,random()%360, 2, YELLOW, BULLET_POINT,0);
                    }
                    break;
            }
            
        }

};
static inline struct enemy *get_enemies(){
    return enemys;
}
static inline void draw_player(int x, int y, int prev_x, int prev_y) {
  // 清除玩家之前的位置
  LCD_Fill(prev_x-2, prev_y-2, prev_x+2, prev_y+2, BLACK); // 清除玩家之前的矩形区域
  // 绘制玩家当前位置
  LCD_DrawRectangle(x-2, y-2, x+2, y+2, GREEN); // 绘制玩家为绿色矩形
}
static inline void move_player(int *x, int *y){
  // Check button presses and update player position
  if (is_button_pressed(JOY_LEFT,50)) {
      if ((*x-2) > 0) // Prevent moving out of bounds
          *x -= 2; // Move left
  }
  if (is_button_pressed(JOY_DOWN,50)) {
      if ((*y+2) < 76) // Prevent moving out of bounds
          *y += 2; // Move down
  }
  if (is_button_pressed(JOY_RIGHT,50)) {
      if ((*x+2) < 160) // Prevent moving out of bounds
          *x += 2; // Move right
  }
  if (is_button_pressed(JOY_UP,50)) {
      if ((*y-2) > 2) // Prevent moving out of bounds
          *y -= 2; // Move up
  }
};
static inline void player_shoot(int x, int y) {
  // Check if the shoot button is pressed
  if (is_button_pressed(BUTTON_1, 200)) {
      // Check if there is space for a new bullet
      if (get_num_bullets() < MAX_POINTS) {
          if (get_num_enemies() == 0) return;

          struct enemy* enemy = get_enemies();
          for (int i = 0; i < MAX_ENEMIES; i++) {
              if (!enemy[i].active) continue; // Skip inactive enemies
              float dx = enemy[i].x - x;
              float dy = enemy[i].y - y;
              float angle_radians = atan2(-dy, dx); 
              unsigned long direction = angle_radians * (180.0 / M_PI); 
              if (direction < 0) {
                  direction += 360;
              }

              add_bullets(x, y, direction, 3, WHITE, BULLET_X, 2); 
              break; // Only shoot at the first active enemy
          }
         
      }
  }
}
extern uint64_t frame_elapsed_time;
uint64_t fps = 0;

// 将函数定义为内联函数
static inline void update_fps() {
    static uint64_t elapsed_time = 0;
    static uint64_t frame_count = 0;
    frame_count++;
    elapsed_time += frame_elapsed_time;

    if (elapsed_time >= 300) {
        fps = (frame_count * 1000) / elapsed_time; 
        LCD_ShowNum(1, 60, fps, 3, WHITE); 
        elapsed_time = 0; 
        frame_count = 0;  
    }
}

static inline void display_fps() {
    LCD_ShowNum(1, 60, fps, 3, WHITE); 
}

static inline void debug() {
    delay_1ms(1000);
    LCD_Clear(BLACK);
    LCD_ShowString(40, 45, (u8*)"DEBUG", WHITE);
    delay_1ms(2000);
}

#endif //CS110_PROJECT4_UTILS_C_H
