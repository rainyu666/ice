#include "logger_config.h"
#include <reent.h> // required for _write_r

#if defined(SEGGER_RTT)
#include "SEGGER_RTT.h"
#else
#include "logger_putchar.h"
#endif

// If necessary define the _reent struct
// to match the one passed by the used standard library.
struct _reent;

_ssize_t _write(int file, const void *ptr, size_t len);
_ssize_t _write_r(struct _reent *r, int file, const void *ptr, size_t len);

_ssize_t _write(int file, const void *ptr, size_t len)
{
    (void) file; // Not used, avoid warning
#if defined(SEGGER_RTT)
    SEGGER_RTT_Write(0, ptr, len);
#else
    const char *p_data = (const char *) ptr;
    for (int i = 0; i < len; i++)
    {
        PUTCHAR(p_data[i]);
    }
#endif
    return len;
}

_ssize_t _write_r(struct _reent *r, int file, const void *ptr, size_t len)
{
    (void) file; // Not used, avoid warning
    (void) r;    // Not used, avoid warning
#if defined(SEGGER_RTT)
    SEGGER_RTT_Write(0, ptr, len);
#else
    const char *p_data = (const char *) ptr;
    for (int i = 0; i < len; i++)
    {
        PUTCHAR(p_data[i]);
    }
#endif
    return len;
}
