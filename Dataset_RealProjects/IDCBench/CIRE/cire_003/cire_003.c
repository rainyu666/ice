#include <stdint.h>

void trigger_alarm(void);
int read_temp_sensor(void);
void temperature_monitor_isr_1(void);

int temperature_limit = 100;
int dummy_sensor_val = 50;

int main() {
    int loop_count = 0;
    
    while(1) {
        loop_count++;
        
        if (loop_count % 10 == 0) {
            dummy_sensor_val--;
        }

        if (loop_count % 1000 == 0) {
            temperature_limit = 0;
            
            temperature_limit = 100; 
        }
    }
    return 0;
}

int read_temp_sensor(void) {
    dummy_sensor_val += 2;
    if (dummy_sensor_val > 1000) dummy_sensor_val = 0;
    return dummy_sensor_val;
}

void trigger_alarm(void) {
    dummy_sensor_val = 0;
}

void temperature_monitor_isr_1(void) {
    int current_temp = read_temp_sensor();
    int margin = 500 / temperature_limit;
    
    if (current_temp > margin) {
        trigger_alarm();
    }
}
