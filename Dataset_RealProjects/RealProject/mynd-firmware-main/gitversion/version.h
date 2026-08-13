#pragma once

#include "gitversion.h"

#define STRINGIFY(s) #s
#define TO_STRING(s) STRINGIFY(s)

#define VERSION_FIRMWARE_STRING TO_STRING(VERSION_MAJOR) "." TO_STRING(VERSION_MINOR) "." TO_STRING(VERSION_PATCH)
#define VERSION_BUILD_STRING    TO_STRING(VERSION_BUILD)

#if VERSION_MAJOR > 100
#define VERSION_FIRMWARE_STRING_SHORT TO_STRING(VERSION_MAJOR)
#else
#define VERSION_FIRMWARE_STRING_SHORT TO_STRING(VERSION_MAJOR) "." TO_STRING(VERSION_MINOR)
#endif // VERSION_MAJOR > 100

#define FW_VERSION_DO_EXPAND(VAL) VAL##1
#define FW_VERSION_EXPAND(VAL)    FW_VERSION_DO_EXPAND(VAL)

inline __attribute__((used)) void __mcu_firmware_version()
{
#if !defined(VERSION_BUILD) || (FW_VERSION_EXPAND(VERSION_BUILD) == 1)
    static const char __srcvers[] __attribute__((used)) = "FW_VERSION:" VERSION_FIRMWARE_STRING;
#else
    static const char __srcvers[] __attribute__((used)) =
        "FW_VERSION:" VERSION_FIRMWARE_STRING "-" VERSION_BUILD_STRING;
#endif
}
