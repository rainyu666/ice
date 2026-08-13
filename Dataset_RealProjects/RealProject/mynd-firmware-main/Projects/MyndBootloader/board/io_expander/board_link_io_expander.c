#include "board_link_io_expander.h"
#include "board_hw.h"
#include "bsp_shared_i2c.h"
#include "aw9523b.h"
#include "logger.h"

#define BT_BUTTON_PORT_PIN    AW9523B_P0_2
#define PLAY_BUTTON_PORT_PIN  AW9523B_P0_3
#define PLUS_BUTTON_PORT_PIN  AW9523B_P0_5
#define MINUS_BUTTON_PORT_PIN AW9523B_P0_4

#define SOURCE_LED_R_PORT_PIN AW9523B_P1_0
#define SOURCE_LED_G_PORT_PIN AW9523B_P1_1
#define SOURCE_LED_B_PORT_PIN AW9523B_P1_2

#define STATUS_LED_R_PORT_PIN AW9523B_P1_6
#define STATUS_LED_G_PORT_PIN AW9523B_P1_5
#define STATUS_LED_B_PORT_PIN AW9523B_P1_4

static struct
{
    aw9523b_handler_t                         *p_handler;
    board_link_io_expander_interrupt_handler_t user_interrupt_handler;
    bool                                       is_initialized;
} s_io_expander;

static const char initialization_error_str[] = "IO expander used before initialization";

static int i2c_write(uint8_t i2c_address, uint8_t register_address, uint8_t *p_buffer, uint16_t length);
static int i2c_read(uint8_t i2c_address, uint8_t register_address, uint8_t *p_buffer, uint16_t length);

static const struct aw9523b_config driver_config = {
    .msp_init     = NULL,
    .msp_deinit   = NULL,
    .i2c_addr     = AW9523B_I2C_ADDRESS,
    .i2c_read     = i2c_read,
    .i2c_write    = i2c_write,
    .mutex        = NULL,
    .mutex_lock   = NULL,
    .mutex_unlock = NULL,
};

static int i2c_write(uint8_t i2c_address, uint8_t register_address, uint8_t *p_buffer, uint16_t length)
{
    return bsp_shared_i2c_write(i2c_address, register_address, (uint8_t *) p_buffer, length);
}

static int i2c_read(uint8_t i2c_address, uint8_t register_address, uint8_t *p_buffer, uint16_t length)
{
    return bsp_shared_i2c_read(i2c_address, register_address, p_buffer, length);
}

void board_link_io_expander_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // Configure IO expander reset pin
    IO_EXP_RESET_GPIO_CLK_ENABLE();
    board_link_io_expander_reset(true);
    GPIO_InitStruct.Pin   = IO_EXP_RESET_GPIO_PIN;
    GPIO_InitStruct.Mode  = IO_EXP_RESET_GPIO_MODE;
    GPIO_InitStruct.Pull  = IO_EXP_RESET_GPIO_PULL;
    GPIO_InitStruct.Speed = IO_EXP_RESET_GPIO_SPEED;
    HAL_GPIO_Init(IO_EXP_RESET_GPIO_PORT, &GPIO_InitStruct);

    // Configure IO expander interrupt pin
    IO_EXP_INT_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin   = IO_EXP_INT_GPIO_PIN;
    GPIO_InitStruct.Mode  = IO_EXP_INT_GPIO_MODE;
    GPIO_InitStruct.Pull  = IO_EXP_INT_GPIO_PULL;
    GPIO_InitStruct.Speed = IO_EXP_INT_GPIO_SPEED;
    HAL_GPIO_Init(IO_EXP_INT_GPIO_PORT, &GPIO_InitStruct);

    s_io_expander.p_handler = aw9523b_init(&driver_config);
    if (s_io_expander.p_handler == NULL)
    {
        log_error("Failed to initialize IO expander");
    }
}

void board_link_io_expander_reset(bool assert)
{
    // Reset pin is active low
    HAL_GPIO_WritePin(IO_EXP_RESET_GPIO_PORT, IO_EXP_RESET_GPIO_PIN, (assert ? GPIO_PIN_RESET : GPIO_PIN_SET));

    if (assert)
    {
        s_io_expander.is_initialized = false;
    }

    log_info("IO expander reset %s", assert ? "asserted" : "deasserted");
}

int board_link_io_expander_setup_for_normal_operation(void)
{
    int result = 0;

    // All pins are configured as GPIO outputs after software reset
    result += aw9523b_software_reset(s_io_expander.p_handler);

    // Configure LED1 GPIOs as LEDs
    result += aw9523b_config_port(s_io_expander.p_handler, AW9523B_PORT1, 0x77, AW9523B_PORT_MODE_LED);

    if (result == 0)
    {
        log_info("IO expander initialized");
        s_io_expander.is_initialized = true;
    }
    else
    {
        log_error("Failed to initialize IO expander");
    }

    return result;
}

int board_link_io_expander_set_status_led(uint8_t r, uint8_t g, uint8_t b)
{
    if (s_io_expander.is_initialized == false)
    {
        log_error("%s", initialization_error_str);
        return -1;
    }

    // The order of RGB pins was inverted
    return aw9523b_set_dimming_multi(s_io_expander.p_handler, STATUS_LED_B_PORT_PIN, (uint8_t[]){b, g, r}, 3);
}

int board_link_io_expander_set_source_led(uint8_t r, uint8_t g, uint8_t b)
{
    if (s_io_expander.is_initialized == false)
    {
        log_error("%s", initialization_error_str);
        return -1;
    }

    return aw9523b_set_dimming_multi(s_io_expander.p_handler, SOURCE_LED_R_PORT_PIN, (uint8_t[]){r, g, b}, 3);
}
