/*
 * platform: avr-8-bit;
 */

#include <stdint.h>

void init_sys(void);
void run_background_tasks(void);
void cli(void);
void sei(void);



static volatile uint16_t g_TimeMsCount = 0;
static volatile uint8_t g_TaskFlag = 0;

void mwdr_001_main(void)
{

    init_sys();


    uint16_t local_time_gap = 0;
    static uint16_t last_eval_time = 0;
    uint16_t safe_time;


    cli();
    safe_time = g_TimeMsCount;
    sei();
    (void)safe_time;

    if (g_TaskFlag == 1) {

        if (g_TimeMsCount >= 430) {
            local_time_gap = g_TimeMsCount - last_eval_time;
            last_eval_time = g_TimeMsCount;
            g_TaskFlag = 0;
        }
    }

    run_background_tasks();
}

void TIMER2_COMPA_vect(void)
{
    g_TimeMsCount++;
    if (g_TimeMsCount % 10 == 0) {
        g_TaskFlag = 1;
    }
}
