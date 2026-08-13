#include <stdint.h>
#include <stdlib.h>
#include "stm32f0xx_hal.h"
#include "board_hw.h"

#define VOLT_TO_12_BIT_CNT(VOLT) ((uint16_t) ((((VOLT) / 3.3f) * 4095.f) + 0.5f))

static const uint16_t hw_revision_id_table[10] = {
    VOLT_TO_12_BIT_CNT(0.3f), VOLT_TO_12_BIT_CNT(0.6f), VOLT_TO_12_BIT_CNT(0.9f), VOLT_TO_12_BIT_CNT(1.2f),
    VOLT_TO_12_BIT_CNT(1.5f), VOLT_TO_12_BIT_CNT(1.8f), VOLT_TO_12_BIT_CNT(2.1f), VOLT_TO_12_BIT_CNT(2.4f),
    VOLT_TO_12_BIT_CNT(2.7f), VOLT_TO_12_BIT_CNT(3.0f)};

static uint8_t adc_single_channel_conversion(ADC_HandleTypeDef *hadc, int adc_channel)
{
    ADC_ChannelConfTypeDef adc_config;

    int32_t  hw_id_adc_val   = 0;
    uint32_t hw_adc_val_diff = 0;
    uint32_t hw_id           = 0;

    // Init ADC in simple polling mode only for HW_REV pin
    hadc->Instance = ADC1;
    HAL_ADC_DeInit(hadc);

    hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc->Init.Resolution     = ADC_RESOLUTION_12B;
    hadc->Init.DataAlign      = ADC_DATAALIGN_RIGHT;
    // Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1)
    hadc->Init.ScanConvMode         = ADC_SCAN_DIRECTION_FORWARD;
    hadc->Init.EOCSelection         = ADC_EOC_SINGLE_CONV;
    hadc->Init.LowPowerAutoWait     = DISABLE;
    hadc->Init.LowPowerAutoPowerOff = DISABLE;
    // Continuous mode disabled to have only 1 conversion at each conversion trig
    hadc->Init.ContinuousConvMode    = DISABLE;
    hadc->Init.DiscontinuousConvMode = DISABLE; /* Parameter discarded because sequencer is disabled */
    // Software start to trig the 1st conversion manually, without external event
    hadc->Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc->Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc->Init.DMAContinuousRequests = ENABLE;
    hadc->Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;

    /*
         Note:
         using the PCLK/4 for ADC -> 12MHz
         tSar + tSmpl -> tConv

         tSar @ 12Bit -> 12.5 CLK_cycles
         tSmpl -> ADC_SAMPLETIME_239CYCLES_5

         (12MHz ^-1) * (12.5 + 239.5) -> ~~~ 21us per conversion
   */

    HAL_ADC_Init(hadc);

    adc_config.Channel      = adc_channel;
    adc_config.Rank         = ADC_RANK_CHANNEL_NUMBER;
    adc_config.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

    HAL_ADC_ConfigChannel(hadc, &adc_config);

    HAL_ADCEx_Calibration_Start(hadc);

    hw_id_adc_val = 0;

    for (uint32_t i = 0; i < 4; i++)
    {
        // Start the conversion process
        HAL_ADC_Start(hadc);

        // Wait for the end of conversion
        if (HAL_ADC_PollForConversion(hadc, 10) == HAL_OK)
        {
            // ADC conversion completed
            hw_id_adc_val += HAL_ADC_GetValue(hadc);
        }
    }

    hw_id_adc_val /= 4;

    // Find the closest ADC count to the measured ADC Value and get the ID from Table;
    hw_adc_val_diff = (uint32_t) abs(hw_id_adc_val - hw_revision_id_table[0]);
    for (uint32_t i = 1; i < 10; i++)
    {
        if ((uint32_t) abs(hw_id_adc_val - hw_revision_id_table[i]) < hw_adc_val_diff)
        {
            hw_adc_val_diff = (uint32_t) abs(hw_id_adc_val - hw_revision_id_table[i]);
            hw_id           = i;
        }
    }

    HAL_ADC_DeInit(hadc);

    return hw_id + 1; // id does not specify V0, it starts with V1
}

static uint8_t hw_board_revision = UINT8_MAX;
static uint8_t hw_bt_revision    = UINT8_MAX;
static uint8_t hw_amp_revision   = UINT8_MAX;

static void update_hw_revisions(void)
{
    ADC_HandleTypeDef hadc;
    GPIO_InitTypeDef  GPIO_InitStruct;

    // Enable ADC peripheral clock
    __HAL_RCC_ADC1_CLK_ENABLE();
    // Enable GPIO clock
    HW_REVISION_GPIO_CLK_ENABLE();
    BT_VER_GPIO_CLK_ENABLE();
    AMP_VER_GPIO_CLK_ENABLE();

    // ADC Channel GPIO pin configuration
    GPIO_InitStruct.Pin  = HW_REVISION_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(HW_REVISION_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BT_VER_GPIO_PIN;
    HAL_GPIO_Init(BT_VER_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = AMP_VER_GPIO_PIN;
    HAL_GPIO_Init(AMP_VER_GPIO_PORT, &GPIO_InitStruct);

    hw_board_revision = adc_single_channel_conversion(&hadc, HW_REVISION_ADC_CHANNEL);
    hw_bt_revision    = adc_single_channel_conversion(&hadc, BT_VER_ADC_CHANNEL);
    hw_amp_revision   = adc_single_channel_conversion(&hadc, AMP_VER_ADC_CHANNEL);

    // Reset peripherals
    __HAL_RCC_ADC1_FORCE_RESET();
    __HAL_RCC_ADC1_RELEASE_RESET();

    // De-initialize the ADC Channel GPIO pins
    HAL_GPIO_DeInit(HW_REVISION_GPIO_PORT, HW_REVISION_GPIO_PIN);
    HAL_GPIO_DeInit(BT_VER_GPIO_PORT, BT_VER_GPIO_PIN);
    HAL_GPIO_DeInit(AMP_VER_GPIO_PORT, AMP_VER_GPIO_PIN);
}

uint8_t read_hw_revision(void)
{
    if (hw_board_revision != UINT8_MAX)
    {
        return hw_board_revision;
    }

    update_hw_revisions();

    return hw_board_revision;
}

uint8_t read_bt_hw_revision(void)
{
    if (hw_bt_revision != UINT8_MAX)
    {
        return hw_bt_revision;
    }

    update_hw_revisions();

    return hw_bt_revision;
}

uint8_t read_amp_hw_revision(void)
{
    if (hw_amp_revision != UINT8_MAX)
    {
        return hw_amp_revision;
    }

    update_hw_revisions();

    return hw_amp_revision;
}
