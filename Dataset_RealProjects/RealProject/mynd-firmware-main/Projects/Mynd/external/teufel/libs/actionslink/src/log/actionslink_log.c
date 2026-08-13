#include <stdarg.h>
#include <stdio.h>
#include "actionslink_log.h"

static actionslink_log_fn_t m_actionslink_log_fn;

void actionslink_log_register(actionslink_log_fn_t log_fn)
{
    m_actionslink_log_fn = log_fn;
}

void actionslink_log(actionslink_log_level_t lvl, const char *format, ...)
{
    if (!m_actionslink_log_fn)
        return;

    va_list args;
    va_start(args, format);
    char buf[128];
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    m_actionslink_log_fn(lvl, buf);
}
