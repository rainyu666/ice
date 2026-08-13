/*
 * platform: generic-32-bit;
 */

#include <stdint.h>

void motor_ctrl_init(void);
void idlerun(void);
void enable_irq(int irq);
void disable_irq(int irq);



static volatile double g_MotorSpeedRPM = 0.0;
static volatile double g_EncoderPulses = 0.0;
static volatile uint32_t g_FaultCode = 0;

void mwdr_005_main(void)
{
    
    motor_ctrl_init();
    

    double current_speed = 0.0;
    uint32_t safe_fault;

    
    
    safe_fault = g_FaultCode;
    

    if (safe_fault != 0) {
        return;
    }

    
    if (g_EncoderPulses > 10.0) {

        current_speed = g_MotorSpeedRPM;

        if (current_speed > 3000.0) {
            g_MotorSpeedRPM = 3000.0;
        }
    }

    idlerun();
}

void encoder_pulse_isr(void)
{
    g_EncoderPulses += 1.0;

    g_MotorSpeedRPM = g_EncoderPulses * 60.0 / 100.0;
}
