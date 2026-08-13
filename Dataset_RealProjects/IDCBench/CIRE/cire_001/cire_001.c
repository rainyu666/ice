#include <stdint.h>
#include <stdio.h>

void system_init(uint8_t *index_ptr, uint8_t start_val);
void write_data(uint8_t data);
void uart_rx_isr_1(void);

uint8_t buffer[16];
uint8_t active_index;
uint32_t uart_error_count;

int main() {
    system_init(&active_index, 0);
    uint32_t loop_counter = 0;
    
    while(1) {
        loop_counter++;
        
        if (active_index < 16) {
            write_data(0xAA); 
        } else {
            active_index = 0;
        }
        
        if (loop_counter % 1000 == 0) {
            uart_error_count++;
        }
    }
    return 0;
}

void system_init(uint8_t *index_ptr, uint8_t start_val) {
    *index_ptr = start_val;
    uart_error_count = 0;
    for (int i = 0; i < 16; i++) {
        buffer[i] = 0;
    }
}

void write_data(uint8_t data) {
    buffer[active_index] = data; 
    active_index++;
}

void uart_rx_isr_1(void) {
    buffer[active_index] = 0xFF;
    active_index++;
}

