#pragma once

#include <cstdint>
#include "actionslink.h"

namespace Teufel::Ux::Audio
{
// clang-format off
struct EcoMode { bool value; };
struct UpdateVolume { uint8_t value; };
struct VolumeLevel { uint8_t value; };
struct RequestSoundIcon {
    actionslink_sound_icon_t sound_icon;
    actionslink_sound_icon_playback_mode_t playback_mode = ACTIONSLINK_SOUND_ICON_PLAYBACK_MODE_PLAY_IMMEDIATELY;
    bool loop_forever = false;
};
struct StopPlayingSoundIcon { actionslink_sound_icon_t sound_icon; };
struct SoundIconsActive { bool value; };
struct BassLevel { int8_t value; };
struct TrebleLevel { int8_t value; };

// clang-format on

// Public API
EcoMode          getProperty(EcoMode *);
VolumeLevel      getProperty(VolumeLevel *);
SoundIconsActive getProperty(SoundIconsActive *);
BassLevel        getProperty(BassLevel *);
TrebleLevel      getProperty(TrebleLevel *);
}
