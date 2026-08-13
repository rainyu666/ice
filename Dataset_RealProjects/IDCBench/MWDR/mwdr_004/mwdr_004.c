/*
 * platform: riscv-32-bit;
 */

#include <stdint.h>

void sys_adc_init(void);
void __enable_irq(void);
void __disable_irq(void);

#define SENSOR_COUNT 4

static volatile uint64_t sensor_accumulators[SENSOR_COUNT];
static volatile uint8_t adc_channels_done = 0;
static volatile uint32_t sample_count = 0;

void mwdr_004_main(void)
{
    __disable_irq();
    sys_adc_init();
    __enable_irq();

    uint64_t total_sum = 0;
    uint32_t safe_count;
    int i;

    __disable_irq();
    safe_count = sample_count;
    __enable_irq();
    (void)safe_count;

    if (adc_channels_done) {
        for (i = 0; i < SENSOR_COUNT; i++) {
            total_sum += sensor_accumulators[i];
        }
        adc_channels_done = 0;
    }
    (void)total_sum;
}

void ADC_IRQHandler(void)
{
    int current_ch = 2;

    sensor_accumulators[current_ch] += 12345ULL;
    sample_count++;
    adc_channels_done = 1;
}
