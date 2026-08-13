#include <stdint.h>

void background_task(void);
void state_change_isr_1(void);

int system_state = 1;
int process_value = 0;
int bg_ticker = 0;

int main() {
    system_state = 1;
    bg_ticker = 0;
    
    while(1) {
        background_task();
        
        if (bg_ticker > 10) {
            bg_ticker = 0;
            system_state = 1; 
        }
    }
    return 0;
}

void background_task(void) {
    int local_state = system_state; 
    
    if (local_state != 0) {
        process_value = 100 / local_state; 
    }
}

void state_change_isr_1(void) {
    system_state = 0;
    bg_ticker++;
}

