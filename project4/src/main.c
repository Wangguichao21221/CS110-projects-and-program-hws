#include "lcd/lcd.h"
#include "utils.h"
#include "Ccode/game.h"
#define DEBOUNCE_DELAY 100
#define NUM_KEYS 7
const char Easystr[] = "1. Easy";
const char Mediumstr[] = "2. Medium";
const char Hardstr[] = "3. Hard";
const char Expertstr[] = "4. Expert";
const char game_startstr[] = "Start after 2s";
const char difficultystr[] = "Difficulty: ";
const char Easy[] = "Easy";
const char Medium[]= "Medium";
const char Hard[] = "Hard";
const char Expert[] = "Expert";


enum {
    JOY_UP_KEY,
    JOY_DOWN_KEY,
    JOY_LEFT_KEY,
    JOY_RIGHT_KEY,
    JOY_CTR_KEY,
    BUTTON_1_KEY,
    BUTTON_2_KEY
};

uint32_t last_trigger_time[NUM_KEYS] = {0};

uint32_t get_current_time_ms(void) {
  uint64_t current_timer_value = get_timer_value();

  uint32_t current_time_ms = (uint32_t)(current_timer_value / (SystemCoreClock / 4000.0));

  return current_time_ms;
}
bool is_button_pressed(int key, int debounce) {
    uint32_t current_time = get_current_time_ms();
    if (Get_Button(key)) {
        if (current_time - last_trigger_time[key] >= debounce) {
            last_trigger_time[key] = current_time;
            return 1; 
        }
    }
    return 0; 
}
void Inp_init(void) {
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOC);

  gpio_init(GPIOA, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ,
            GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
  gpio_init(GPIOC, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ,
            GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
}

void IO_init(void) {
  Inp_init(); // inport init
  Lcd_Init(); // LCD init
}

int main(void) {
  IO_init();
  uint32_t difficulty = scenario_selection();
  scenario_display(difficulty);
  run_game(difficulty);
}
