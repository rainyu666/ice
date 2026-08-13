#pragma once

#include <stdbool.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    /**
     * @brief Initializes the hardware needed to interface the boost converter.
     */
    void board_link_boost_converter_init(void);

    /**
     * @brief Enables/disables the boost converter.
     *
     * @param[in] enable        enable option
     */
    void board_link_boost_converter_enable(bool enable);

#if defined(__cplusplus)
}
#endif
