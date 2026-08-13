/*
 * platform: arm-32-bit;
 */

#include <stdint.h>

void drv_uart_init(void);
void __disable_irq(void);
void __enable_irq(void);

static volatile uint32_t uart_rx_count = 0;
static volatile uint64_t global_utc_ms = 0;

void mwdr_003_main(void)
{
    __disable_irq();
    drv_uart_init();
    __enable_irq();

    uint64_t local_timestamp;
    uint32_t safe_rx_count;

    __disable_irq();
    safe_rx_count = uart_rx_count;
    __enable_irq();
    (void)safe_rx_count;

    local_timestamp = global_utc_ms;

    if (local_timestamp > 1000000ULL) {
        uart_rx_count = 0;
    }
}

void SysTick_Handler(void)
{
    global_utc_ms++;
}
