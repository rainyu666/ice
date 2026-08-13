#pragma once

#include <variant>
#include <initializer_list>

#include "ux/system/system.h"
#include "ux/audio/audio.h"
#include "ux/bluetooth/bluetooth.h"
#include "ux/input/input.h"

#include "task_audio.h"

namespace Teufel::Task::System
{

// clang-format off
using SystemMessage = std::variant<
    Teufel::Ux::System::SetPowerState,
    Teufel::Ux::System::UserActivity,
    Teufel::Ux::System::OffTimer,
    Teufel::Ux::System::OffTimerEnabled,
    Teufel::Ux::System::FactoryResetRequest
>;
// clang-format on

Teufel::Ux::System::PowerState getState();

int start();
int postMessage(Teufel::Ux::System::Task source_task, SystemMessage msg);
}
