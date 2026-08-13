#pragma once

#if defined(TEUFEL_LOGGER)

#include "logger.h"

#define dev_fatal(...)      log_fatal(__VA_ARGS__)
#define dev_err(...)        log_error(__VA_ARGS__)
#define dev_warn(...)       log_warning(__VA_ARGS__)
#define dev_high(...)       log_highlight(__VA_ARGS__)
#define dev_info(...)       log_info(__VA_ARGS__)
#define dev_dbg(...)        log_debug(__VA_ARGS__)
#define dev_trace(...)      log_trace(__VA_ARGS__)

#define dev_fatal_raw(...)  log_fatal_raw(__VA_ARGS__)
#define dev_err_raw(...)    log_error_raw(__VA_ARGS__)
#define dev_warn_raw(...)   log_warning_raw(__VA_ARGS__)
#define dev_high_raw(...)   log_highlight_raw(__VA_ARGS__)
#define dev_info_raw(...)   log_info_raw(__VA_ARGS__)
#define dev_dbg_raw(...)    log_debug_raw(__VA_ARGS__)
#define dev_trace_raw(...)  log_trace_raw(__VA_ARGS__)

#define prompt_driver_init_ok(x)     dev_info("[%s] Init OK", x);
#define prompt_driver_init_failed(x) dev_err("[%s] Init FAILED!", x);

#elif defined(TEUFEL_LOG_TRACE) || \
    defined(TEUFEL_LOG_DEBUG) || \
    defined(TEUFEL_LOG_INFO) || \
    defined(TEUFEL_LOG_WARN) ||  \
    defined(TEUFEL_LOG_ERROR)

#include "logger.h"

#if defined(DEVTRACE)
#define dev_err(fmt, ...)            log_err(fmt, ##__VA_ARGS__);
#define dev_warn(fmt, ...)           log_warn(fmt, ##__VA_ARGS__);
#define dev_info(fmt, ...)           log_info(fmt, ##__VA_ARGS__);
#define dev_dbg(fmt, ...)            log_dbg(fmt, ##__VA_ARGS__);
#define dev_trace(fmt, ...)          log_trace(fmt, ##__VA_ARGS__);

#define dev_errt(fmt, ...)           log_errt(fmt, ##__VA_ARGS__);
#define dev_warnt(fmt, ...)          log_warnt(fmt, ##__VA_ARGS__);
#define dev_infot(fmt, ...)          log_infot(fmt, ##__VA_ARGS__);
#define dev_dbgt(fmt, ...)           log_dbgt(fmt, ##__VA_ARGS__);
#define dev_tracet(fmt, ...)         log_tracet(fmt, ##__VA_ARGS__);

#define dev_err_raw(fmt, ...)        log_err_raw(fmt, ##__VA_ARGS__);
#define dev_warn_raw(fmt, ...)       log_warn_raw(fmt, ##__VA_ARGS__);
#define dev_info_raw(fmt, ...)       log_info_raw(fmt, ##__VA_ARGS__);
#define dev_dbg_raw(fmt, ...)        log_dbg_raw(fmt, ##__VA_ARGS__);
#define dev_trace_raw(fmt, ...)      log_trace_raw(fmt, ##__VA_ARGS__);

#define prompt_driver_init_ok(x)     log_info("[%s] Init OK", x);
#define prompt_driver_init_failed(x) log_err("[%s] Init FAILED!", x);
#elif defined(DEVDEBUG)
#define dev_err(fmt, ...)            log_err(fmt, ##__VA_ARGS__);
#define dev_warn(fmt, ...)           log_warn(fmt, ##__VA_ARGS__);
#define dev_info(fmt, ...)           log_info(fmt, ##__VA_ARGS__);
#define dev_dbg(fmt, ...)            log_dbg(fmt, ##__VA_ARGS__);
#define dev_trace(fmt, ...)

#define dev_errt(fmt, ...)           log_errt(fmt, ##__VA_ARGS__);
#define dev_warnt(fmt, ...)          log_warnt(fmt, ##__VA_ARGS__);
#define dev_infot(fmt, ...)          log_infot(fmt, ##__VA_ARGS__);
#define dev_dbgt(fmt, ...)           log_dbgt(fmt, ##__VA_ARGS__);
#define dev_tracet(fmt, ...)

#define dev_err_raw(fmt, ...)        log_err(fmt, ##__VA_ARGS__);
#define dev_warn_raw(fmt, ...)       log_warn(fmt, ##__VA_ARGS__);
#define dev_info_raw(fmt, ...)       log_info(fmt, ##__VA_ARGS__);
#define dev_dbg_raw(fmt, ...)        log_dbg(fmt, ##__VA_ARGS__);
#define dev_trace_raw(fmt, ...)

#define prompt_driver_init_ok(x)     log_info("[%s] Init OK", x);
#define prompt_driver_init_failed(x) log_err("[%s] Init FAILED!", x);
#elif defined(DEVINFO)
#define dev_err(fmt, ...)            log_err(fmt, ##__VA_ARGS__);
#define dev_warn(fmt, ...)           log_warn(fmt, ##__VA_ARGS__);
#define dev_info(fmt, ...)           log_info(fmt, ##__VA_ARGS__);
#define dev_dbg(fmt, ...)
#define dev_trace(fmt, ...)

#define dev_errt(fmt, ...)           log_errt(fmt, ##__VA_ARGS__);
#define dev_warnt(fmt, ...)          log_warnt(fmt, ##__VA_ARGS__);
#define dev_infot(fmt, ...)          log_infot(fmt, ##__VA_ARGS__);
#define dev_dbgt(fmt, ...)
#define dev_tracet(fmt, ...)

#define dev_err_raw(fmt, ...)        log_err_raw(fmt, ##__VA_ARGS__);
#define dev_warn_raw(fmt, ...)       log_warn_raw(fmt, ##__VA_ARGS__);
#define dev_info_raw(fmt, ...)       log_info_raw(fmt, ##__VA_ARGS__);
#define dev_dbg_raw(fmt, ...)
#define dev_trace_raw(fmt, ...)

#define prompt_driver_init_ok(x)     log_info("[%s] Init OK", x);
#define prompt_driver_init_failed(x) log_err("[%s] Init FAILED!", x);
#else
#define dev_err(fmt, ...)            log_err(fmt, ##__VA_ARGS__);
#define dev_warn(fmt, ...)           log_warn(fmt, ##__VA_ARGS__);
#define dev_info(fmt, ...)           log_info(fmt, ##__VA_ARGS__);
#define dev_dbg(fmt, ...)
#define dev_trace(fmt, ...)

#define dev_errt(fmt, ...)           log_errt(fmt, ##__VA_ARGS__);
#define dev_warnt(fmt, ...)          log_warnt(fmt, ##__VA_ARGS__);
#define dev_infot(fmt, ...)          log_infot(fmt, ##__VA_ARGS__);
#define dev_dbgt(fmt, ...)
#define dev_tracet(fmt, ...)

#define dev_err_raw(fmt, ...)        log_err_raw(fmt, ##__VA_ARGS__);
#define dev_warn_raw(fmt, ...)       log_warn_raw(fmt, ##__VA_ARGS__);
#define dev_info_raw(fmt, ...)       log_info_raw(fmt, ##__VA_ARGS__);
#define dev_dbg_raw(fmt, ...)
#define dev_trace_raw(fmt, ...)

#define prompt_driver_init_ok(x)     dev_info("[%s] Init OK", x);
#define prompt_driver_init_failed(x) dev_err("[%s] Init FAILED!", x);
#endif

#else // TEUFEL_USE_LOGGER not defined

#include <stdio.h>

#define dev_err(fmt, ...)            printf(fmt, ##__VA_ARGS__);
#define dev_warn(fmt, ...)           printf(fmt, ##__VA_ARGS__);
#define dev_info(fmt, ...)           printf(fmt, ##__VA_ARGS__);
#define dev_dbg(fmt, ...)
#define dev_trace(fmt, ...)

#define dev_err_raw(fmt, ...)        printf(fmt, ##__VA_ARGS__);
#define dev_warn_raw(fmt, ...)       printf(fmt, ##__VA_ARGS__);
#define dev_info_raw(fmt, ...)       printf(fmt, ##__VA_ARGS__);
#define dev_dbg_raw(fmt, ...)
#define dev_trace_raw(fmt, ...)

#define prompt_driver_init_ok(x)     dev_info("[%s] Init OK", x);
#define prompt_driver_init_failed(x) dev_err("[%s] Init FAILED!", x);

#endif // defined(TEUFEL_USE_LOGGER)
