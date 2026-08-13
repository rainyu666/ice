#include <stdlib.h>
#include <stdbool.h>

#define LOG_MODULE_NAME  "aw9523b.c"
#define LOG_LEVEL        LOG_LEVEL_INFO
#include "driver_logger.h"

#include "aw9523b.h"

#define REG_INPUT_PORT0              0x0U
#define REG_INPUT_PORT1              0x1U
#define REG_OUTPUT_PORT0             0x2U
#define REG_OUTPUT_PORT1             0x3U
#define REG_CONFIG_PORT0             0x4U
#define REG_CONFIG_PORT1             0x5U
#define REG_INT_PORT0                0x6U
#define REG_INT_PORT1                0x7U
#define REG_ID                       0x10U
#define REG_GCR                      0x11U // Global control register
#define REG_LED_MODE_SWITCH1         0x12U // Switch for the Port1
#define REG_LED_MODE_SWITCH2         0x13U // Switch for the Port2
#define REG_DIMMING_CONTROL_PORT_1_0 0x20U
#define REG_SOFTWARE_RESET           0x7FU

static const char *log_prefix = "aw9523b";

struct aw9523b_handler
{
    int (*msp_init)(void);   // Optional
    int (*msp_deinit)(void); // Optional
    int (*i2c_read)(uint8_t addr, uint8_t subaddr, uint8_t *data, uint16_t len);
    int (*i2c_write)(uint8_t addr, uint8_t subaddr, uint8_t *data, uint16_t len);
    void (*mutex_lock)(void *mutex);   // Optional
    void (*mutex_unlock)(void *mutex); // Optional
    void   *mutex;                     // Optional
    uint8_t i2c_addr;
    uint8_t port0_cached;
    uint8_t port1_cached;
};

static inline bool is_valid_port(aw9523b_port_t port)
{
    return (port == AW9523B_PORT0 || port == AW9523B_PORT1);
}

static inline void mutex_try_lock(const struct aw9523b_handler *h)
{
    if (h->mutex && h->mutex_lock && h->mutex_unlock)
        h->mutex_lock(h->mutex);
}

static inline void mutex_try_unlock(const struct aw9523b_handler *h)
{
    if (h->mutex && h->mutex_unlock && h->mutex_unlock)
        h->mutex_unlock(h->mutex);
}

static int rmw_reg(const struct aw9523b_handler *h, uint8_t reg, uint8_t mask, uint8_t data)
{
    static uint8_t reg_data;
    int            ret;

    mutex_try_lock(h);

    ret = h->i2c_read(h->i2c_addr, reg, &reg_data, 1);
    if (ret < 0)
    {
        mutex_try_unlock(h);
        dev_err("%s: read reg %02X failed", log_prefix, reg);
        return -E_AW9523B_IO;
    }

    reg_data &= ~mask;
    reg_data |= data;

    ret = h->i2c_write(h->i2c_addr, reg, &reg_data, 1);
    if (ret < 0)
    {
        mutex_try_unlock(h);
        dev_err("%s: write reg %02X failed", log_prefix, reg);
        return -E_AW9523B_IO;
    }

    mutex_try_unlock(h);
    return E_AW9523B_OK;
}

struct aw9523b_handler *aw9523b_init(const struct aw9523b_config *conf)
{
    if (!conf)
    {
        dev_err("%s: config is not defined", log_prefix);
        return NULL;
    }

    if (!conf->i2c_read || !conf->i2c_write)
    {
        dev_err("%s: missing callback functions", log_prefix);
        prompt_driver_init_failed(log_prefix);
        return NULL;
    }

    struct aw9523b_handler *h = malloc(sizeof(struct aw9523b_handler));
    if (!h)
    {
        dev_err("%s: malloc", log_prefix);
        prompt_driver_init_failed(log_prefix);
        return NULL;
    }

    h->msp_init     = conf->msp_init;
    h->msp_deinit   = conf->msp_deinit;
    h->i2c_read     = conf->i2c_read;
    h->i2c_write    = conf->i2c_write;
    h->i2c_addr     = conf->i2c_addr;
    h->mutex        = conf->mutex;
    h->mutex_lock   = conf->mutex_lock;
    h->mutex_unlock = conf->mutex_unlock;

    h->port0_cached = 0;
    h->port1_cached = 0;

    if (h->msp_init)
        h->msp_init();

    prompt_driver_init_ok(log_prefix);

    return h;
}

int aw9523b_software_reset(const struct aw9523b_handler *h)
{
    if (!h)
        return -E_AW9523B_PARAM;

    if (h->i2c_write(h->i2c_addr, REG_SOFTWARE_RESET, (uint8_t[]){0U}, 1) < 0)
        return -E_AW9523B_IO;
    return E_AW9523B_OK;
}

int aw9523b_set_port0(const struct aw9523b_handler *h, aw9523b_gpio_drive_mode_t mode)
{
    if (mode == AW9523B_PUSH_PULL)
    {
        if (h->i2c_write(h->i2c_addr, REG_GCR, (uint8_t[]){0x10U}, 1) < 0)
            return -E_AW9523B_IO;
    }
    return E_AW9523B_OK;
}

int aw9523b_config_port(const struct aw9523b_handler *h, aw9523b_port_t port, uint8_t pin_mask,
                        aw9523b_gpio_port_mode_t mode)
{
    uint8_t data = 0;

    if (!h)
        return -E_AW9523B_PARAM;

    if (!is_valid_port(port))
        return -E_AW9523B_PARAM;

    if (mode != AW9523B_PORT_MODE_LED && mode != AW9523B_PORT_MODE_GPIO)
        return -E_AW9523B_PARAM;

    uint8_t reg = port == AW9523B_PORT0 ? REG_LED_MODE_SWITCH1 : REG_LED_MODE_SWITCH2;

    // If a bit in this register is set to 1, the corresponding port pin is configured as GPIO.
    if (mode == AW9523B_PORT_MODE_GPIO)
        data |= pin_mask;

    return rmw_reg(h, reg, pin_mask, data);
}

int aw9523b_set_direction(const struct aw9523b_handler *h, aw9523b_port_t port, uint8_t pin_mask,
                          aw9523b_port_direction_t direction)
{
    uint8_t data = 0;

    if (!h)
        return -E_AW9523B_PARAM;

    if (!is_valid_port(port))
        return -E_AW9523B_PARAM;

    uint8_t reg = port == AW9523B_PORT0 ? REG_CONFIG_PORT0 : REG_CONFIG_PORT1;

    // If a bit in this register is set to 1, the corresponding port pin is enabled
    // as an input with a high-impedance output driver.
    if (direction == AW9523B_PORT_DIRECTION_INPUT)
        data |= pin_mask;

    return rmw_reg(h, reg, pin_mask, data);
}

int aw9523b_port_interrupt_enable(const struct aw9523b_handler *h, aw9523b_port_t port, uint8_t pin_mask)
{
    if (!h)
        return -E_AW9523B_PARAM;

    if (!is_valid_port(port))
        return -E_AW9523B_PARAM;

    uint8_t reg = port == AW9523B_PORT0 ? REG_INT_PORT0 : REG_INT_PORT1;

    // If a bit in this register is set to 1, the interrupt function
    // of the corresponding port pin is disabled.
    return rmw_reg(h, reg, pin_mask, 0x00);
}

int aw9523b_port_interrupt_disable(const struct aw9523b_handler *h, aw9523b_port_t port, uint8_t pin_mask)
{
    if (!h)
        return -E_AW9523B_PARAM;

    if (!is_valid_port(port))
        return -E_AW9523B_PARAM;

    uint8_t reg = port == AW9523B_PORT0 ? REG_INT_PORT0 : REG_INT_PORT1;

    // If a bit in this register is set to 1, the interrupt function
    // of the corresponding port pin is disabled.
    return rmw_reg(h, reg, pin_mask, 0xff);
}

int aw9523b_write_pin(const struct aw9523b_handler *h, uint8_t port, uint8_t pin, uint8_t value)
{
    uint8_t data = 0;

    if (!h)
        return -E_AW9523B_PARAM;

    if (!is_valid_port(port) || pin > 7)
        return -E_AW9523B_PARAM;

    uint8_t reg = port == AW9523B_PORT0 ? REG_OUTPUT_PORT0 : REG_OUTPUT_PORT1;

    if (value)
        data |= (1U << pin);

    return rmw_reg(h, reg, (1U << pin), data);
}

int aw9523b_read_pin(const struct aw9523b_handler *h, uint8_t port, uint8_t pin)
{
    int     err;
    uint8_t data;

    if (!h)
        return -E_AW9523B_PARAM;

    if (!is_valid_port(port) || pin > 7)
        return -E_AW9523B_PARAM;

    uint8_t reg = port == AW9523B_PORT0 ? REG_INPUT_PORT0 : REG_INPUT_PORT1;

    // mutex_try_lock(h);
    err = h->i2c_read(h->i2c_addr, reg, &data, 1);
    // mutex_try_unlock(h);

    if (err < 0)
        return -E_AW9523B_IO;

    return (data >> pin) & 0x1;
}

int aw9523b_write_pin_cache(struct aw9523b_handler *h, uint8_t port, uint8_t pin, uint8_t value)
{
    uint8_t reg = 0;
    int     ret;

    if (!h)
        return -E_AW9523B_PARAM;

    if (!is_valid_port(port) || pin > 7)
        return -E_AW9523B_PARAM;

    if (port == AW9523B_PORT0)
    {
        reg = REG_OUTPUT_PORT0;
        if (value)
            h->port0_cached |= (1U << pin);
        else
            h->port0_cached &= ~(1U << pin);
        ret = h->i2c_write(h->i2c_addr, reg, &h->port0_cached, 1);
    }
    else
    {
        reg = REG_OUTPUT_PORT1;
        if (value)
            h->port1_cached |= (1U << pin);
        else
            h->port1_cached &= ~(1U << pin);
        ret = h->i2c_write(h->i2c_addr, reg, &h->port1_cached, 1);
    }

    if (ret < 0)
    {
        dev_err("%s: write reg %02X failed", log_prefix, reg);
        return -E_AW9523B_IO;
    }

    return E_AW9523B_OK;
}

int aw9523b_read_pin_cache(const struct aw9523b_handler *h, uint8_t port, uint8_t pin)
{
    if (!h)
        return -E_AW9523B_PARAM;

    if (!is_valid_port(port) || pin > 7)
        return -E_AW9523B_PARAM;

    uint8_t data = 0;
    if (port == AW9523B_PORT0)
        data = h->port0_cached;
    else
        data = h->port1_cached;

    return (data >> pin) & 0x1;
}

int aw9523b_set_dimming(const struct aw9523b_handler *h, uint8_t port, uint8_t pin, uint8_t value)
{
    uint8_t reg = 0;

    if (!h)
        return -E_AW9523B_PARAM;

    if (!is_valid_port(port) || pin > 7)
        return -E_AW9523B_PARAM;

    if (port == AW9523B_PORT0)
    {
        reg = 0x24u + pin;
    }
    else
    {
        if (pin <= 3)
            reg = 0x20u + pin;
        else
            reg = 0x2Cu + pin - 4;
    }

    if (h->i2c_write(h->i2c_addr, reg, (uint8_t[]){value}, 1) < 0)
        return -E_AW9523B_IO;

    return 0;
}

int aw9523b_set_dimming_multi(const struct aw9523b_handler *h, uint8_t port, uint8_t pin, const uint8_t *value,
                              uint8_t count)
{
    uint8_t reg = 0;

    if (!h)
        return -E_AW9523B_PARAM;

    if (!is_valid_port(port) || pin > 7)
        return -E_AW9523B_PARAM;

    if (port == AW9523B_PORT0)
    {
        reg = 0x24u + pin;
    }
    else
    {
        if (pin <= 3)
            reg = 0x20u + pin;
        else
            reg = 0x2Cu + pin - 4;
    }

    if (h->i2c_write(h->i2c_addr, reg, (uint8_t *) value, count) < 0)
        return -E_AW9523B_IO;

    return 0;
}

int aw9523b_set_current(const struct aw9523b_handler *h, aw9523b_current_t value)
{
    if (!h)
        return -E_AW9523B_PARAM;

    return rmw_reg(h, REG_GCR, 0x3, (uint8_t)value);
}

