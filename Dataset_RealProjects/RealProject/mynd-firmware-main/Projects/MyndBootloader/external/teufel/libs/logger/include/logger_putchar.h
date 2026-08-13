#pragma once

#ifdef __GNUC__

#define PUTCHAR(c)        __io_putchar(c)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)

PUTCHAR_PROTOTYPE;

#else

#define PUTCHAR(c)        fputc(c, __stdout)
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

PUTCHAR_PROTOTYPE;

#endif // __GNUC__
