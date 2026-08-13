#pragma once

#include <stdint.h>

#define E_AW9523B_OK     0
#define E_AW9523B_PARAM  1 // Invalid param
#define E_AW9523B_IO     2 // I/O operation failed
#define E_AW9523B_MALLOC 3 // Memory allocation error

// Port/Pin aliases
#define AW9523B_P0_0 0U, 0U
#define AW9523B_P0_1 0U, 1U
#define AW9523B_P0_2 0U, 2U
#define AW9523B_P0_3 0U, 3U
#define AW9523B_P0_4 0U, 4U
#define AW9523B_P0_5 0U, 5U
#define AW9523B_P0_6 0U, 6U
#define AW9523B_P0_7 0U, 7U

#define AW9523B_P1_0 1U, 0U
#define AW9523B_P1_1 1U, 1U
#define AW9523B_P1_2 1U, 2U
#define AW9523B_P1_3 1U, 3U
#define AW9523B_P1_4 1U, 4U
#define AW9523B_P1_5 1U, 5U
#define AW9523B_P1_6 1U, 6U
#define AW9523B_P1_7 1U, 7U

struct aw9523b_config
{
    int (*msp_init)(void);   // Optional
    int (*msp_deinit)(void); // Optional
    int (*i2c_read)(uint8_t addr, uint8_t subaddr, uint8_t *data, uint16_t len);
    int (*i2c_write)(uint8_t addr, uint8_t subaddr, uint8_t *data, uint16_t len);
    void (*mutex_lock)(void *mutex);   // Optional
    void (*mutex_unlock)(void *mutex); // Optional
    void   *mutex;                     // Optional
    uint8_t i2c_addr;
};

typedef enum
{
    AW9523B_OPEN_DRAIN,
    AW9523B_PUSH_PULL,
} aw9523b_gpio_drive_mode_t;

typedef enum
{
    AW9523B_PORT_MODE_LED,
    AW9523B_PORT_MODE_GPIO,
} aw9523b_gpio_port_mode_t;

typedef enum
{
    AW9523B_PORT0,
    AW9523B_PORT1,
} aw9523b_port_t;

typedef enum
{
    AW9523B_PORT_DIRECTION_INPUT,
    AW9523B_PORT_DIRECTION_OUTPUT,
} aw9523b_port_direction_t;

typedef enum
{
    AW9523B_CURRENT_MAX,
    AW9523B_CURRENT_3_4, // IMAX * 3/4
    AW9523B_CURRENT_2_4, // IMAX * 2/4
    AW9523B_CURRENT_1_4, // IMAX * 1/4
} aw9523b_current_t;

typedef struct aw9523b_handler aw9523b_handler_t;

#if defined(__cplusplus)
extern "C"
{
#endif

    aw9523b_handler_t *aw9523b_init(const struct aw9523b_config *conf);

    /**
     * @brief Software reset
     * @param[in] h control handler
     * @return 0 if reset  successfully, -1,-2,-3,... otherwise
     */
    int aw9523b_software_reset(const aw9523b_handler_t *h);

    /**
     * Configure Port0 via Global control register.
     * @param[in] h control handler
     * @param mode OPEN_DRAIN(default) or PUSH_PULL mode
     * @return 0 if reset  successfully, -1,-2,-3,... otherwise
     */
    int aw9523b_set_port0(const struct aw9523b_handler *h, aw9523b_gpio_drive_mode_t mode);

    /**
     * @brief Configure the port as LED or GPIO mode.
     * @param[in] h control handler
     * @param[in] port target port
     * @param[in] pin_mask bit mask of the pins to configuration
     * @param[in] mode LED or GPIO mode
     * @return 0 if configuration set successfully, -1,-2,-3,... otherwise
     */
    int aw9523b_config_port(const aw9523b_handler_t *h, aw9523b_port_t port, uint8_t pin_mask,
                            aw9523b_gpio_port_mode_t mode);

    /**
     * @brief Configure the port direction. Set INPUT or OUTPUT direction.
     * @param[in] h control handler
     * @param[in] port target port
     * @param[in] pin_mask bit mask of the pins to configuration
     * @param[in] direction INPUT or OUTPUT direction
     * @return 0 if direction set successfully, -1,-2,-3,... otherwise
     */
    int aw9523b_set_direction(const aw9523b_handler_t *h, aw9523b_port_t port, uint8_t pin_mask,
                              aw9523b_port_direction_t direction);

    /**
     * @brief Enable interrupt for a certain port.
     * @param[in] h control handler
     * @param[in] port target port
     * @param[in] pin_mask bit mask of the pins to enable interrupt
     * @return 0 if interrupt was enabled successfully, -1,-2,-3,... otherwise
     */
    int aw9523b_port_interrupt_enable(const aw9523b_handler_t *h, aw9523b_port_t port, uint8_t pin_mask);

    /**
     * @brief Disable interrupt for a certain port.
     * @param[in] h control handler
     * @param[in] port target port
     * @param[in] pin_mask bit mask of the pins to enable interrupt
     * @return 0 if interrupt was disabled successfully, -1,-2,-3,... otherwise
     */
    int aw9523b_port_interrupt_disable(const aw9523b_handler_t *h, aw9523b_port_t port, uint8_t pin_mask);

    /**
     * @brief Read input state of a certain port.
     * @param[in] h control handler
     * @param[in] port target port
     * @param[out] data input state of the port
     * @return input state of the port, -1,-2,-3,... otherwise
     */
    int aw9523b_read_port(const struct aw9523b_handler *h, aw9523b_port_t port, uint8_t *data);

    /**
     * @brief Write output state of a certain port.
     * @param[in] h control handler
     * @param[in] port target port
     * @param[in] pin target pin
     * @param[in] value 0U or !0U pin state
     * @return 0 if direction set successfully, -1,-2,-3,... otherwise
     * NOTE: the type of the argument port is uint8_t, in order to simplify
     * to use aliases: AW9523B_P0_0, AW9523B_P0_1, etc. w/o casting.
     */
    int aw9523b_write_pin(const aw9523b_handler_t *h, uint8_t port, uint8_t pin, uint8_t value);

    /**
     * @brief Read output status of a certain port.
     * @param[in] h control handler
     * @param[in] port target port
     * @param[in] pin target pin
     * @return 0 or 1 as an input state of a pin, -1,-2,-3,... otherwise
     * NOTE: the type of the argument port is uint8_t, in order to simplify
     * to use aliases: AW9523B_P0_0, AW9523B_P0_1, etc. w/o casting.
     */
    int aw9523b_read_pin(const aw9523b_handler_t *h, uint8_t port, uint8_t pin);

    /**
     * @brief Write output state of a certain port and save cached value.
     * @details The function doesn't use read-modify-write approach but adjust cached value.
     * @param[in] h control handler
     * @param[in] port target port
     * @param[in] pin target pin
     * @param[in] value 0U or !0U pin state
     * @return 0 if direction set successfully, -1,-2,-3,... otherwise
     * NOTE: the type of the argument port is uint8_t, in order to simplify
     * to use aliases: AW9523B_P0_0, AW9523B_P0_1, etc. w/o casting.
     */
    int aw9523b_write_pin_cache(aw9523b_handler_t *h, uint8_t port, uint8_t pin, uint8_t value);

    /**
     * @brief Read output status of a certain port.
     * @details The function doesn't read the value from the IC, but return the cached value.
     * @param[in] h control handler
     * @param[in] port target port
     * @param[in] pin target pin
     * @return 0 or 1 as an input state of a pin, -1,-2,-3,... otherwise
     * NOTE: the type of the argument port is uint8_t, in order to simplify
     * to use aliases: AW9523B_P0_0, AW9523B_P0_1, etc. w/o casting.
     */
    int aw9523b_read_pin_cache(const aw9523b_handler_t *h, uint8_t port, uint8_t pin);

    /**
     * @brief Set dimming control register.
     * @param[in] h control handler
     * @param[in] port target port
     * @param[in] pin target pin
     * @param[in] value dimming value (0-255)
     * @return 0 or 1 as an input state of a pin, -1,-2,-3,... otherwise
     * NOTE: the type of the argument port is uint8_t, in order to simplify
     * to use aliases: AW9523B_P0_0, AW9523B_P0_1, etc. w/o casting.
     */
    int aw9523b_set_dimming(const aw9523b_handler_t *h, uint8_t port, uint8_t pin, uint8_t value);

    int aw9523b_set_dimming_multi(const aw9523b_handler_t *h, uint8_t port, uint8_t pin, const uint8_t *value,
                                  uint8_t count);

    /**
     * @brief Set current for all ports.
     * @param h control handler
     * @param value current value
     * @return 0 if current set successfully, -1,-2,-3,... otherwise
     */
    int aw9523b_set_current(const aw9523b_handler_t *h, aw9523b_current_t value);

#if defined(__cplusplus)
}
#endif
