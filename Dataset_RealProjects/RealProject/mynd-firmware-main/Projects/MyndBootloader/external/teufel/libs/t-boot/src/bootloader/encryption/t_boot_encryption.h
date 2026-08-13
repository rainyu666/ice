#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Decodes classic Vigenere's cipher with fixed length alphabetic key to an output buffer.
 *
 * @param[in] p_input           pointer to input buffer
 * @param[in] input_length      length of input buffer
 * @param[in] p_key             pointer to encryption key
 * @param[in] key_length        length of encryption key
 * @param[out] p_output         pointer to output buffer
 */
void t_boot_encryption_decode(const uint8_t *p_input, size_t input_length, const uint8_t *p_key, size_t key_length, uint8_t *p_output);

/**
 * @brief Decodes classic Vigenere's cipher in-place with fixed length alphabetic key.
 *
 * @param[inout] p_input        pointer to input buffer
 * @param[in] input_length      length of input buffer
 * @param[in] p_key             pointer to encryption key
 * @param[in] key_length        length of encryption key
 */
void t_boot_encryption_decode_in_place(uint8_t *p_input, size_t input_length, const uint8_t *p_key, size_t key_length);
