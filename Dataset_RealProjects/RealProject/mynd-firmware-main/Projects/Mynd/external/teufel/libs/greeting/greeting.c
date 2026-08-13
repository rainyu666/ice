#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef FreeRTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

#ifdef STM32F0

#include "stm32f0xx_hal.h"
// #include "core_cm0.h"
#elif STM32F1
#include "stm32f1xx_hal.h"
#elif STM32F3
#include "stm32f3xx_hal.h"
#elif STM32F4
#include "stm32f4xx_hal.h"
#endif

#ifdef IMX8MM
#include "MIMX8MM6_cm4.h"
#endif

#ifdef IMX8MN
#include "MIMX8MN6_cm7.h"
#endif

#ifdef IMXRT1011
#include "MIMXRT1011.h"
#endif

#include "gitversion/version.h"

void PrintFirmwareInfo(const char *name)
{
    printf("       _______          __     _\r\n \
     |__   __|        / _|   | |\r\n \
        | | ___ _   _| |_ ___| |\r\n \
        | |/ _ \\ | | |  _/ _ \\ |\r\n \
        | |  __/ |_| | ||  __/ |\r\n \
        |_|\\___|\\__,_|_| \\___|_|\r\n\r\n");

    printf(" - Firmware:\t\tv%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    if (strlen(VERSION_BUILD_STRING) != 0)
    {
        printf("-%s (%s %s)\r\n", VERSION_BUILD_STRING, __DATE__, __TIME__);
    }
    else
    {
        printf("\r\n");
    }
#if ((defined IMX8MM) || (defined IMX8MN))
    printf(" - CMSIS Core:\t\t%d.%d\r\n", __CM_CMSIS_VERSION_MAIN, __CM_CMSIS_VERSION_SUB);
#endif
#if (defined STM32F0) || (defined STM32F1) || (defined STM32F3) || (defined STM32F4)
    uint32_t hal_version = HAL_GetHalVersion();
#if (defined STM32F0)
    printf(" - CMSIS Core:\t\t%d.%d\r\n", __CM0_CMSIS_VERSION_MAIN, __CM0_CMSIS_VERSION_SUB);
#elif (defined STM32F3)
    printf(" - CMSIS Core:\t\t%d.%d\r\n", __CM3_CMSIS_VERSION_MAIN, __CM3_CMSIS_VERSION_SUB);
#elif (defined STM32F4)
    printf(" - CMSIS Core:\t\t%d.%d\r\n", __CM4_CMSIS_VERSION_MAIN, __CM4_CMSIS_VERSION_SUB);
#endif
    printf(" - STM HAL lib:\t\tv%ld.%ld.%ld\r\n", (hal_version >> 24U), (hal_version >> 16U) & 0x000000ff,
           (hal_version >> 8U) & 0x000000ff);
#endif
#if (defined IMXRT1011)
    printf(" - CMSIS Core:\t\t%d.%d\r\n", __CM_CMSIS_VERSION_MAIN, __CM_CMSIS_VERSION_SUB);
#endif
#ifdef FreeRTOS
    printf(" - FreeRTOS:\t\tv%d.%d.%d\r\n", tskKERNEL_VERSION_MAJOR, tskKERNEL_VERSION_MINOR, tskKERNEL_VERSION_BUILD);
#endif
    // printf(" - Rpmsg Lite: v%s\r\n", RL_VERSION);
    // PRINTF(" - Rfp:        v%d.%d.%d\r\n", RFP_VERSION_MAJOR, RFP_VERSION_MINOR);
    // printf(" - Dsp Build:\t\t%s\r\n", DspBuildName);
    printf(" - Hardware:\t\t%s\r\n", name);
    printf("\r\n\r\n");
}
