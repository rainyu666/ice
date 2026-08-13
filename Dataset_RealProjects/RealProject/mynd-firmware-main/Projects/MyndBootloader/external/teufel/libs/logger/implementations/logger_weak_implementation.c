#include <stdint.h>

__attribute__((weak)) void logger_flush(void) {}

__attribute__((weak)) uint32_t logger_get_timestamp()
{
    return 0;
}
