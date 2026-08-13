#include <stdio.h>
#include <stdint.h>
#include <cmsis_gcc.h>

#include "app_assert.h"

void app_assertion_handler(const char *file, int line_number)
{
    printf("Assertion failed in %s:%d\r\n", file, line_number);
    __disable_irq();
    __BKPT(0);
    for (;;)
    {
    }
}

void __assert_func(const char *file, int line, const char *func, const char *failedExpr)
{
    printf("Assertion failed in %s:%d\r\n", file, line);
    printf("%s in %s()\r\n", failedExpr, func);
    __disable_irq();
    __BKPT(0);
    for (;;)
    {
    }
}
