#include "bsp_adc.h"
#include "board_hw.h"
#include "board_link_hw_revision.h"
#include "stm32f0xx_hal.h"

#define LOG_LEVEL LOG_LEVEL_ERROR
#include "logger.h"
#include "external/teufel/libs/app_assert/app_assert.h"

ADC_HandleTypeDef        Adc1Handle;
static DMA_HandleTypeDef DmaHandle;

static bsp_adc_conversion_complete_callback_t s_user_callback;

void bsp_bat_voltage_enable_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    BAT_VOLTAGE_ENABLE_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin   = BAT_VOLTAGE_ENABLE_GPIO_PIN;
    GPIO_InitStruct.Mode  = BAT_VOLTAGE_ENABLE_GPIO_MODE;
    GPIO_InitStruct.Pull  = BAT_VOLTAGE_ENABLE_GPIO_PULL;
    GPIO_InitStruct.Speed = BAT_VOLTAGE_ENABLE_GPIO_SPEED;
    HAL_GPIO_Init(BAT_VOLTAGE_ENABLE_GPIO_PORT, &GPIO_InitStruct);
}

void bsp_bat_voltage_enable(bool enable)
{
    if (enable)
        HAL_GPIO_WritePin(BAT_VOLTAGE_ENABLE_GPIO_PORT, BAT_VOLTAGE_ENABLE_GPIO_PIN, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(BAT_VOLTAGE_ENABLE_GPIO_PORT, BAT_VOLTAGE_ENABLE_GPIO_PIN, GPIO_PIN_RESET);
}

void bsp_adc_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    ISENS_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin  = ISENS_GPIO_PIN;
    GPIO_InitStruct.Mode = ISENS_GPIO_MODE;
    GPIO_InitStruct.Pull = ISENS_GPIO_PULL;
    HAL_GPIO_Init(ISENS_GPIO_PORT, &GPIO_InitStruct);

    ISNS_REF_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin  = ISNS_REF_GPIO_PIN;
    GPIO_InitStruct.Mode = ISNS_REF_GPIO_MODE;
    GPIO_InitStruct.Pull = ISNS_REF_GPIO_PULL;
    HAL_GPIO_Init(ISNS_REF_GPIO_PORT, &GPIO_InitStruct);

    PSYS_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin  = PSYS_GPIO_PIN;
    GPIO_InitStruct.Mode = PSYS_GPIO_MODE;
    GPIO_InitStruct.Pull = PSYS_GPIO_PULL;
    HAL_GPIO_Init(PSYS_GPIO_PORT, &GPIO_InitStruct);

    BAT_NTC2_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin  = BAT_NTC2_GPIO_PIN;
    GPIO_InitStruct.Mode = BAT_NTC2_GPIO_MODE;
    GPIO_InitStruct.Pull = BAT_NTC2_GPIO_PULL;
    HAL_GPIO_Init(BAT_NTC2_GPIO_PORT, &GPIO_InitStruct);

    if (read_hw_revision() >= 5u)
    {
        // Enable the battery voltage measurement (ADC channel)
        BAT_VOLTAGE_GPIO_CLK_ENABLE();
        GPIO_InitStruct.Pin  = BAT_VOLTAGE_GPIO_PIN;
        GPIO_InitStruct.Mode = BAT_VOLTAGE_GPIO_MODE;
        GPIO_InitStruct.Pull = BAT_VOLTAGE_GPIO_PULL;
        HAL_GPIO_Init(BAT_VOLTAGE_GPIO_PORT, &GPIO_InitStruct);
    }

    // Enable clock of ADCx peripheral
    __HAL_RCC_ADC1_CLK_ENABLE();
    // Enable DMA1 clock
    __HAL_RCC_DMA1_CLK_ENABLE();

    // clang-format off
    // Configure DMA parameters
    DmaHandle.Instance                 = DMA1_Channel1;
    DmaHandle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    DmaHandle.Init.PeriphInc           = DMA_PINC_DISABLE;
    DmaHandle.Init.MemInc              = DMA_MINC_ENABLE;
    DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;   // ADC resolution is 12 bits, so we need to transfer 16 bits
    DmaHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;   // ADC resolution is 12 bits, so we need to transfer 16 bits
    DmaHandle.Init.Mode                = DMA_CIRCULAR;              // Circular mode to support ADC DMA Continuous Requests
    DmaHandle.Init.Priority            = DMA_PRIORITY_LOW;
    // clang-format on

    // Deinitialize  & Initialize the DMA for new transfer
    HAL_DMA_DeInit(&DmaHandle);
    HAL_DMA_Init(&DmaHandle);

    // Associate the DMA handle
    __HAL_LINKDMA(&Adc1Handle, DMA_Handle, DmaHandle);

    // NVIC configuration for DMA Input data interrupt
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    // Initialize ADC peripheral
    Adc1Handle.Instance                   = ADC1;
    Adc1Handle.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4; // PCLK = 48 MHz, so ADC CLK = 12 MHz
    Adc1Handle.Init.LowPowerAutoWait      = DISABLE;
    Adc1Handle.Init.LowPowerAutoPowerOff  = DISABLE;
    Adc1Handle.Init.Resolution            = ADC_RESOLUTION_12B;
    Adc1Handle.Init.ScanConvMode          = ADC_SCAN_DIRECTION_FORWARD;
    Adc1Handle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    Adc1Handle.Init.ScanConvMode          = ENABLE;
    Adc1Handle.Init.ContinuousConvMode    = ENABLE;
    Adc1Handle.Init.DiscontinuousConvMode = DISABLE;
    Adc1Handle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    Adc1Handle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    Adc1Handle.Init.EOCSelection          = ADC_EOC_SEQ_CONV;
    Adc1Handle.Init.DMAContinuousRequests = ENABLE;
    Adc1Handle.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;
    Adc1Handle.Init.SamplingTimeCommon    = ADC_SAMPLETIME_239CYCLES_5;

    HAL_ADC_DeInit(&Adc1Handle);
    // Initialize ADC peripheral according to the passed parameters
    if (HAL_ADC_Init(&Adc1Handle) != HAL_OK)
    {
        APP_ASSERT(false, "Failed to initialize ADC");
    }

    // Start calibration
    HAL_ADCEx_Calibration_Start(&Adc1Handle);

    HAL_NVIC_SetPriority(ADC1_COMP_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ADC1_COMP_IRQn);
}

void bsp_adc_deinit(void)
{
    // TODO: Verify that this makes sense, copied from UltimaSoundbar
    if (Adc1Handle.State != HAL_ADC_STATE_RESET)
    {
        HAL_ADC_Stop_DMA(&Adc1Handle);

        ADC_ChannelConfTypeDef channel_config;
        channel_config.Channel      = ISENS_ADC_CHANNEL;
        channel_config.Rank         = ADC_RANK_NONE;
        channel_config.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;

        HAL_ADC_ConfigChannel(&Adc1Handle, &channel_config);
    }
}

void bsp_adc_start(uint32_t *buffer, uint32_t buffer_size, bsp_adc_conversion_complete_callback_t callback)
{
    HAL_StatusTypeDef      status;
    ADC_ChannelConfTypeDef sConfig;
    s_user_callback = callback;

    HAL_ADC_Stop_DMA(&Adc1Handle);

    // Sampling time is common to all channels
    // The total conversion time is t_conv = (t_smpl + t_sar) * (t_adcclk)
    // t_sar for 12-bit is 12.5 ADC clock cycles
    // t_smpl is 239.5 ADC clock cycles
    // t_adcclk is 1/12 MHz = 83.3 ns
    // So t_conv = (239.5 + 12.5) * 83.3 ns = 21 us per conversion
    //
    // The ADC is triggered by TIM15, which has a period of 100 ms,
    // so the sampling time is not a concern when reading less than 4000 samples or so
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

    // The data will be stored in the buffer according to the channel number
    // For example if we are sampling 3 channels (0-2), the data in the buffer will be written like this:
    // [<0>, <1>, <2>, <0>, <1>, <2>, <0>, <1>, <2>, ...]
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;

    sConfig.Channel = ISENS_ADC_CHANNEL;
    status          = HAL_ADC_ConfigChannel(&Adc1Handle, &sConfig);
    APP_ASSERT(status == HAL_OK);

    sConfig.Channel = ISNS_REF_ADC_CHANNEL;
    status          = HAL_ADC_ConfigChannel(&Adc1Handle, &sConfig);
    APP_ASSERT(status == HAL_OK);

    sConfig.Channel = PSYS_ADC_CHANNEL;
    status          = HAL_ADC_ConfigChannel(&Adc1Handle, &sConfig);
    APP_ASSERT(status == HAL_OK);

    sConfig.Channel = BAT_NTC2_ADC_CHANNEL;
    status          = HAL_ADC_ConfigChannel(&Adc1Handle, &sConfig);
    APP_ASSERT(status == HAL_OK);

    if (read_hw_revision() >= 5u)
    {
        sConfig.Channel = BAT_VOLTAGE_ADC_CHANNEL;
        status          = HAL_ADC_ConfigChannel(&Adc1Handle, &sConfig);
        APP_ASSERT(status == HAL_OK);
    }

    sConfig.Channel = ADC_CHANNEL_VREFINT;
    status          = HAL_ADC_ConfigChannel(&Adc1Handle, &sConfig);
    APP_ASSERT(status == HAL_OK);

    HAL_ADC_Start_DMA(&Adc1Handle, buffer, buffer_size);
}

void bsp_adc_stop(void)
{
    HAL_ADC_Stop_DMA(&Adc1Handle);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    (void) hadc;
    if (s_user_callback != NULL)
    {
        s_user_callback();
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    log_error("ADC error code 0x%02X", hadc->ErrorCode);
    bsp_adc_stop();
}
