#pragma once

#ifdef __cplusplus
extern "C" {
#endif


void app_assertion_handler(const char * file, int line_number);

#if defined(APP_ASSERT_NDEBUG)

#define APP_ASSERT(...)

#else

#define APP_ASSERT(condition, ...)                           \
    do {                                                     \
        if ((condition) == 0) {                              \
            app_assertion_handler(__FILE_NAME__, __LINE__);  \
        }                                                    \
    } while (0)

#endif // defined(APP_ASSERT_NDEBUG)

#ifdef __cplusplus
}
#endif
