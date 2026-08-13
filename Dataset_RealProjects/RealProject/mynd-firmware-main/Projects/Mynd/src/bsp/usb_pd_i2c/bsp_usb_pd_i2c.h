#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    /**
     * @brief Initializes the I2C hardware needed to interface with the USB PD I2C bus.
     */
    void bsp_usb_pd_i2c_init(void);

    void bsp_usb_pd_i2c_msp_init(void);

    /**
     * @brief Writes to the USB PD I2C bus.
     *
     * @param[in] i2c_address           I2C device address
     * @param[in] register_address      register address to write to
     * @param[in] p_buffer              pointer to data to write
     * @param[in] length                number of bytes to write
     *
     * @return 0 if successful, -1 otherwise
     */
    int bsp_usb_pd_i2c_write(uint8_t i2c_address, uint8_t register_address, const uint8_t *p_buffer, uint32_t length);

    /**
     * @brief Reads from the USB PD I2C bus.
     *
     * @param[in]  i2c_address          I2C device address
     * @param[in]  register_address     register address to read from
     * @param[out] p_buffer             pointer to where the read data will be written to
     * @param[in]  length               number of bytes to read
     *
     * @return 0 if successful, -1 otherwise
     */
    int bsp_usb_pd_i2c_read(uint8_t i2c_address, uint8_t register_address, uint8_t *p_buffer, uint32_t length);

#if defined(__cplusplus)
}
#endif
