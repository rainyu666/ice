#include <stdint.h>
#define NULL ((void *)0)

struct Node {
    int id;
    float dummy_metadata;
};

struct Node main_node_1 = {1, 10.0};
struct Node main_node_2 = {2, 20.0};
struct Node* active_node = &main_node_1;

int cleanup_count = 0;

void cleanup_task(void);
void reset_isr_1_1(void);

int main() {
    int loop_counter = 0;
    
    while(1) {
        if (active_node != NULL) {
            cleanup_task();
        }
        
        loop_counter++;
        if (loop_counter % 50 == 0) {
            active_node = &main_node_2;
        }
        
        if (cleanup_count > 1000) {
            cleanup_count = 0;
        }
    }
    return 0;
}

void cleanup_task(void) {
    int my_id = active_node->id;
    active_node->dummy_metadata = 0.0;
    cleanup_count++;
}

void reset_isr_1_1(void) {
    active_node = NULL;
}
