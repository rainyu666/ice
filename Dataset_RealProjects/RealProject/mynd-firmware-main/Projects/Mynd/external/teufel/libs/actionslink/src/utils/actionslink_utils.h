#pragma once

#include "actionslink_types.h"

/**
 * @brief Initializes the actionslink utilities
 *
 * @param[in] p_config      pointer to driver configuration
 */
void actionslink_utils_init(const actionslink_config_t *p_config);

/**
 * @brief Gets the current system timestamp in milliseconds.
 *
 * @return system timestamp in milliseconds
 */
uint32_t actionslink_utils_get_ms(void);

/**
 * @brief Gets the number of milliseconds elapsed since a given timestamp.
 *
 * @param[in] timestamp_ms      timestamp in milliseconds
 *
 * @return number of milliseconds
 */
uint32_t actionslink_utils_get_ms_since(uint32_t timestamp_ms);
