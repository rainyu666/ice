#include <stdint.h>

void motor_init(void);
void compute_control_signal(void);
void motor_control_isr_1(void);
void fault_handling_isr_2(void);

uint32_t motor_faults;
float speed_factor;
float target_speed;
float current_speed;

int main() {
    motor_init();
    
    while(1) {
        if (motor_faults > 5) {
            motor_init();
        }
        
        if (current_speed > 200.0) {
            target_speed = 50.0;
        }
    }
    return 0;
}

void motor_init(void) {
    speed_factor = 1.0;
    current_speed = 0.0;
    motor_faults = 0;
    target_speed = 100.0;
}

void compute_control_signal(void) {
    float control = target_speed / speed_factor;
    current_speed += control;
}

void motor_control_isr_1(void) {
    compute_control_signal();
}

void fault_handling_isr_2(void) {
    motor_faults++;
    speed_factor = 0.0;
}

