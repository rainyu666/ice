#pragma once

#include "FreeRTOS.h"
#include "task.h"

#define TASK_AUDIO_PRIORITY     (tskIDLE_PRIORITY + 1)
#define TASK_BLUETOOTH_PRIORITY (tskIDLE_PRIORITY + 2)
#define TASK_SYSTEM_PRIORITY    (tskIDLE_PRIORITY + 1)
