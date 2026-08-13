#include <stdint.h>

void system_watchdog_feed(void);
void apply_system_mode(void);
void user_input_isr_1(void);
void system_update_isr_2(void);

int config_values[5] = {10, 20, 30, 40, 50};
int active_mode = 0;
int status_flags = 0;
int watchdog_timer = 0;

int main() {
    active_mode = 0;
    status_flags = 0;
    
    while(1) {
        watchdog_timer++;
        
        if(watchdog_timer > 10000) {
            system_watchdog_feed();
        }
        
        if (status_flags == 0xFF) {
            active_mode = 0;
        }
    }
    return 0;
}

void system_watchdog_feed(void) {
    watchdog_timer = 0;
}

void apply_system_mode(void) {
    int current_val = config_values[active_mode];
    status_flags = current_val;
}

void user_input_isr_1(void) {
    status_flags |= 0x01;
    
    active_mode = 5;
    
    active_mode = 0;
}

void system_update_isr_2(void) {
    apply_system_mode();
}
