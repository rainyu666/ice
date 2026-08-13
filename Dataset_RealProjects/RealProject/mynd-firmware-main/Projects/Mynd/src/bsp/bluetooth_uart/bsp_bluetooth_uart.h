#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    /**
     * @brief Initializes the UART hardware needed to interface with the Bluetooth module.
     */
    void bsp_bluetooth_uart_init(void);

    void bsp_bluetooth_uart_msp_init(void);

    void bsp_bluetooth_uart_msp_deinit(void);

    void bsp_bluetooth_uart_clear_buffer(void);

    /**
     * @brief Sends data over UART.
     * @note  This function returns immediately. The TX done callback provided
     *        in `board_hal_bluetooth_uart_init()` will be called when the transfer completes.
     *
     * @param[in] p_data    pointer to data to send
     * @param[in] length    length of data to send
     *
     * @return 0 if successful, -1 otherwise
     */
    int bsp_bluetooth_uart_tx(const uint8_t *p_data, size_t length);

    /**
     * @brief Reads data from the UART RX buffer.
     *
     * @details This function checks if the requested length has already been received and
     *          returns it if that's the case. If the expected length is not in the buffer yet,
     *          the function will return an error.
     *
     * @param[out] p_data               pointer to where the read data will be written
     * @param[in]  length               number of bytes to read
     *
     * @return 0 if successful, -1 otherwise
     */
    int bsp_bluetooth_uart_rx(uint8_t *p_data, size_t length);

#if defined(__cplusplus)
}
#endif
