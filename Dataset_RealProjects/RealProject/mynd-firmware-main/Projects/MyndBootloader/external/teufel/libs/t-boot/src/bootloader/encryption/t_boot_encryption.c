#include <stdbool.h>
#include "t_boot_encryption.h"

__attribute__((always_inline)) static inline bool is_alpha_upper(char c)
{
    return ((c >= 0x41) && (c <= 0x5a));
}

__attribute__((always_inline)) static inline bool is_alpha_lower(char c)
{
    return ((c >= 0x61) && (c <= 0x7a));
}

void t_boot_encryption_decode(const uint8_t *p_input, size_t input_length, const uint8_t *p_key, size_t key_length, uint8_t *p_output)
{
    int8_t shifts[key_length];
    int8_t sign = -1; // Decrypting, shifts will be negative.

    for (size_t i = 0; i < key_length; i++)
    {
        char charcase = is_alpha_upper(p_key[i]) ? 'A' : 'a';
        shifts[i] = (p_key[i] - charcase) * sign;
    }

    for (size_t i = 0, j = 0; i < input_length; i++)
    {
        // Skip non-alphabetical characters
        if ((!is_alpha_upper(p_input[i])) && (!(is_alpha_lower(p_input[i]))))
        {
            p_output[i] = p_input[i];
            continue;
        }

        uint8_t charcase = is_alpha_upper(p_input[i]) ? 'A' : 'a';

        // Wrapping conversion algorithm
        // TODO: Guard against p_output buffer overflow
        p_output[i] = ((p_input[i] + shifts[j] - charcase + 26) % 26) + charcase;

        j = (j + 1) % key_length;
    }
}

void t_boot_encryption_decode_in_place(uint8_t *p_input, size_t input_length, const uint8_t *p_key, size_t key_length)
{
    int8_t shifts[key_length];
    int8_t sign = -1; // Decrypting, shifts will be negative.

    for (size_t i = 0; i < key_length; i++)
    {
        char charcase = is_alpha_upper(p_key[i]) ? 'A' : 'a';

        shifts[i] = (p_key[i] - charcase) * sign;
    }

    for (size_t i = 0, j = 0; i < input_length; i++)
    {
        if ((!is_alpha_upper(p_input[i])) && (!(is_alpha_lower(p_input[i]))))
        {
            continue;
        }

        uint8_t charcase = is_alpha_upper(p_input[i]) ? 'A' : 'a';

        // Wrapping conversion algorithm
        p_input[i] = (((uint8_t)(p_input[i] + shifts[j] - charcase + 26)) % 26) + charcase;

        j = (j + 1) % key_length;
    }
}
