#include <stdbool.h>

#include "gitversion/version.h"
#include "external/teufel/libs/app_assert/app_assert.h"

#include "stm32f0xx_hal.h"

#include "logger.h"

#include "dfu_mcu.h"
#include "t_boot_dfu.h"

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_msc.h"
#include "usbd_storage.h"

#include "board_hw.h"

#include "bsp/shared_i2c/bsp_shared_i2c.h"
#include "bsp/debug_uart/bsp_debug_uart.h"
#include "board/io_expander/board_link_io_expander.h"
#include "board/power_supply//board_link_power_supply.h"
#include "board/usb_switch/board_link_usb_switch.h"

extern UART_HandleTypeDef UART2_Handle;

int __io_putchar(int ch)
{
    HAL_UART_Transmit(&UART2_Handle, (uint8_t *) &ch, 1, HAL_MAX_DELAY);
    return ch;
}

USBD_HandleTypeDef USBD_Device;

volatile enum {
    UPDATE_STATE_IDLE,
    UPDATE_STATE_PREPARE,
    UPDATE_STATE_PROGRESS,
    UPDATE_STATE_COMPLETE,
    UPDATE_STATE_FAIL,
} update_state = UPDATE_STATE_IDLE;

static void SystemClock_Config(void);

volatile uint32_t *VectorTable = (uint32_t *) 0x20000000;

void jump_to_application(const uint32_t address)
{
    typedef void (*p_function)(void);

    uint32_t   stack_pointer;
    p_function app_entry;

    stack_pointer = (uint32_t) * ((__IO uint32_t *) address);
    app_entry     = (p_function) * ((__IO uint32_t *) (address + 4));

    log_err("jump address: 0x%x", address);
    log_err("stack pointer: 0x%x", stack_pointer);
    HAL_Delay(10);

    if (((*(__IO uint32_t *) address) & 0x2FFE0000) == 0x20000000)
    {
        HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
        HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
        HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
        HAL_NVIC_DisableIRQ(SysTick_IRQn);
        // HAL_NVIC_DisableIRQ(-5);

        /* Copy the vector table from the Flash (mapped at the base of the application
        load address) to the base address of the SRAM at 0x20000000. */
        for (uint8_t i = 0; i < 48; i++)
        {
            VectorTable[i] = *(__IO uint32_t *) (address + (i << 2));
        }

        __HAL_RCC_AHB_FORCE_RESET();
        __HAL_RCC_SYSCFG_CLK_ENABLE();
        __HAL_RCC_AHB_RELEASE_RESET();
        /* Remap SRAM at 0x00000000 */
        __HAL_SYSCFG_REMAPMEMORY_SRAM();

        /* disabled systick, this avoids running into hard fault handler in the main application, because osTick
         * handling is called before kernel runs properly */
        SysTick->CTRL &= ~1;

        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t *) APPLICATION_FLASH_ADDRESS);

        app_entry();
    }
    else
    {
        log_err("Firmware has no valid start entry");

        while (1)
            ;
    }
}

static const t_boot_dfu_target_t t_boot_dfu_target_list[] = {{
    .name         = "MCU",
    .component_id = T_BOOT_DFU_COMPONENT_ID_MCU,
    .init         = dfu_mcu_init,
    .prepare      = dfu_mcu_prepare,
    .write        = dfu_mcu_write,
    .verify       = NULL,
    .get_crc32    = NULL,
}};

static void on_dfu_start(uint8_t number_of_components)
{
    (void) number_of_components;
    log_info("DFU on_start");
}

static void on_dfu_progress_update(t_boot_dfu_component_id_t component_id, uint32_t progress)
{
    log_info("DFU progress: %d %d", component_id, progress);
}

static void on_dfu_component_done(t_boot_dfu_component_id_t component_id)
{
    (void) component_id;
    log_info("DFU component done: %d", component_id);
}

static void on_dfu_complete(void)
{
    log_info("DFU complete");
    update_state = UPDATE_STATE_COMPLETE;
}

static void on_dfu_failure(t_boot_dfu_component_id_t component_id, int error_code)
{
    log_info("DFU failure");
    update_state = UPDATE_STATE_FAIL;
}

static const t_boot_config_t t_boot_config = {
    .p_dfu_target_list        = t_boot_dfu_target_list,
    .dfu_target_list_size     = sizeof(t_boot_dfu_target_list) / sizeof(t_boot_dfu_target_t),
    .update_start_fn          = on_dfu_start,
    .update_progress_fn       = on_dfu_progress_update,
    .update_component_done_fn = on_dfu_component_done,
    .update_successful_fn     = on_dfu_complete,
    .update_error_fn          = on_dfu_failure,
};

#define LEDS_OFF   0x00, 0x00, 0x00
#define LEDS_RED   0xFF, 0x00, 0x00
#define LEDS_GREEN 0x00, 0xFF, 0x00

struct indication_pattern
{
    uint8_t  led_value;
    uint16_t duration_ms;
};

static struct indication_pattern i_pattern[] = {{1, 200}, {0, 200}, {1, 200}, {0, 200}, {0, 1000}};

static void non_block_indication(void)
{
    static uint32_t tick_ms = 0u;
    static int      index   = 0;

    if (tick_ms == 0)
    {
        index   = 0;
        tick_ms = get_systick();
    }

    if (board_get_ms_since(tick_ms) > i_pattern[index].duration_ms)
    {
        tick_ms = get_systick();
        if (index >= sizeof(i_pattern) / sizeof(struct indication_pattern))
        {
            tick_ms = 0;
        }
        if (i_pattern[index].led_value == 1)
        {
            board_link_io_expander_set_status_led(LEDS_RED);
        }
        else
        {
            board_link_io_expander_set_status_led(LEDS_OFF);
        }
        index = (index + 1) % (sizeof(i_pattern) / sizeof(struct indication_pattern));
    }
}

static void periodic_task(void)
{
    if (update_state == UPDATE_STATE_IDLE || update_state == UPDATE_STATE_PROGRESS)
    {
        non_block_indication();
    }

    if (update_state == UPDATE_STATE_FAIL)
    {
        board_link_io_expander_set_status_led(LEDS_RED);

        if (board_link_power_supply_button_is_pressed())
        {
            NVIC_SystemReset();
        }
    }

    if (update_state == UPDATE_STATE_COMPLETE)
    {
        // Magic # for power on after update signal to application
        RTC->BKP0R = 0xBEEFBEEF;

        board_link_io_expander_set_status_led(LEDS_GREEN);
        USBD_Stop(&USBD_Device);

        // Wait for USB to be disconnected and show the green LED for a bit
        HAL_Delay(3000);
        NVIC_SystemReset();
    }
}

static void run_bootloader(void)
{
    USBD_StatusTypeDef status = USBD_OK;

    board_link_power_supply_init();
    board_link_power_supply_hold_on(true);

    bsp_shared_i2c_init();
    board_link_io_expander_init();
    board_link_io_expander_reset(false);

    HAL_Delay(2);

    board_link_io_expander_setup_for_normal_operation();

    t_boot_dfu_init(&t_boot_config);

    board_link_usb_switch_init();

    board_link_usb_switch_to_mcu();

    /* Init Device Library */
    if (USBD_Init(&USBD_Device, &MSC_Desc, 0) != USBD_OK)
        goto usb_err;

    /* Register the MSC class */
    if (USBD_RegisterClass(&USBD_Device, &USBD_MSC) != USBD_OK)
        goto usb_err;

    /* Add Storage callbacks for MSC Class */
    if (USBD_MSC_RegisterStorage(&USBD_Device, &USBD_DISK_fops) != USBD_OK)
        goto usb_err;

    /* Start Device Process */
    status = USBD_Start(&USBD_Device);
    if (status != USBD_OK)
        goto usb_err;

    log_info("USB started");

    while (1)
    {
        static uint32_t tick = 0;
        if (get_systick() - tick > 10)
        {
            tick = get_systick();
            periodic_task();
        }
    }

usb_err:
    log_err("USB error (TODO: handle it!");
    APP_ASSERT(false);
}

static bool is_application_valid(void)
{
    volatile const uint32_t *app_vector    = (volatile const uint32_t *) APPLICATION_FLASH_ADDRESS;
    uint32_t                 stack_pointer = app_vector[0];
    uint32_t                 app_entry     = app_vector[1];

    log_trace("app addr 0x%x - 0x%x", APPLICATION_FLASH_ADDRESS, APPLICATION_FLASH_ADDRESS + APPLICATION_FLASH_SIZE);
    log_trace("stack pointer: 0x%x, app_entry: 0x%x", stack_pointer, app_entry);

    // Stack pointer should be in RAM region
    if (((*(__IO uint32_t *) APPLICATION_FLASH_ADDRESS) & 0x2FFE0000) != 0x20000000)
    {
        return false;
    }

    if ((app_entry < APPLICATION_FLASH_ADDRESS) || (app_entry >= APPLICATION_FLASH_ADDRESS + APPLICATION_FLASH_SIZE))
    {
        log_error("Invalid app entry point 0x%08X", app_entry);
        return false;
    }

    log_highlight("Application valid: App entry 0x%08X", app_entry);
    return true;
}

bool should_start_bootloader(void)
{
    if (is_application_valid() == false)
    {
        log_err("Application invalid");
        return true;
    }

    // Enable backup registers to examine the reset reason
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
    {
        // Power on reset
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    {
        // Check Softreset reason in Backup registers
        uint32_t bkup_0 = RTC->BKP0R;
        log_high("backup register: 0x%x", bkup_0);
        if (bkup_0 == 0xCAFEBEEF)
        {
            log_debug("Application requested bootloader");
            RTC->BKP0R = 0;
            return true;
        }
        else if (bkup_0 == 0x0)
        {
            RTC->BKP0R = 0;
            return false;
        }
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        // Independent watchdog reset
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
    {
        // Low power reset
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
    {
        // Window watchdog reset
    }

    __HAL_RCC_CLEAR_RESET_FLAGS();

    return false;
}

uint32_t logger_get_timestamp()
{
    return get_systick();
}

int main(void)
{
    HAL_Init();

    __HAL_RCC_SYSCFG_CLK_ENABLE();

    SystemClock_Config();

    bsp_debug_uart_init();

    const char *version = "\r\nMYNDBootloader - v" VERSION_FIRMWARE_STRING "\r\n";

    for (int i = 0; i < strlen(version); i++)
    {
        __io_putchar(version[i]);
    }

    if (should_start_bootloader())
    {
        run_bootloader();
    }
    else
    {
        log_info("Starting application...");
        jump_to_application(APPLICATION_FLASH_ADDRESS);
    }
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSI48)
 *            SYSCLK(Hz)                     = 48000000
 *            HCLK(Hz)                       = 48000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 1
 *            HSI Frequency(Hz)              = 48000000
 *            PREDIV                         = 2
 *            PLLMUL                         = 2
 *            Flash Latency(WS)              = 1
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef       RCC_ClkInitStruct;
    RCC_OscInitTypeDef       RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /* Select HSI48 Oscillator as PLL source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
    RCC_OscInitStruct.HSI48State     = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSI48;
    RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL2;
    RCC_OscInitStruct.PLL.PREDIV     = RCC_PREDIV_DIV2;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* Initializes the CPU, AHB and APB busses clocks */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB | RCC_PERIPHCLK_I2C1;
    PeriphClkInit.UsbClockSelection    = RCC_USBCLKSOURCE_HSI48;
    PeriphClkInit.I2c1ClockSelection   = RCC_I2C1CLKSOURCE_SYSCLK;

    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

    /* Configure the Systick interrupt time */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    /* Configure the Systick */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* Set USB FS Interrupt priority */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
