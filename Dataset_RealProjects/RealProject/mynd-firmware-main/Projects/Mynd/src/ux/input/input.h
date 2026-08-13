#pragma once

#include <cstdint>
#include <optional>

namespace Teufel::Ux
{
enum class InputId : uint8_t
{
    Power,
    BT,
    Plus,
    Minus,
    PlayPause,
};

inline auto getDesc(const InputId &value)
{
    switch (value)
    {
        case InputId::Power:
            return "Power";
        case InputId::BT:
            return "BT";
        case InputId::Plus:
            return "Plus";
        case InputId::Minus:
            return "Minus";
        case InputId::PlayPause:
            return "PlayPause";
        default:
            return "Unknown";
    }
}

enum class InputState : uint8_t
{
    ShortPress,
    ShortRelease,
    MediumPress,
    MediumRelease,
    LongPress,
    LongRelease,
    VeryLongPress,
    VeryLongRelease,
    VeryVeryLongPress,
    VeryVeryLongRelease,
    DoublePress,
    DoubleRelease,
    TriplePress,
    TripleRelease,
    Hold,
    RawPress,
    RawRelease,
};

inline auto getDesc(const InputState &value)
{
    switch (value)
    {
        case InputState::ShortPress:
            return "ShortPress";
        case InputState::ShortRelease:
            return "ShortRelease";
        case InputState::MediumPress:
            return "MediumPress";
        case InputState::MediumRelease:
            return "MediumRelease";
        case InputState::LongPress:
            return "LongPress";
        case InputState::LongRelease:
            return "LongRelease";
        case InputState::VeryLongPress:
            return "VeryLongPress";
        case InputState::VeryLongRelease:
            return "VeryLongRelease";
        case InputState::VeryVeryLongPress:
            return "VeryVeryLongPress";
        case InputState::VeryVeryLongRelease:
            return "VeryVeryLongRelease";
        case InputState::DoublePress:
            return "DoublePress";
        case InputState::DoubleRelease:
            return "DoubleRelease";
        case InputState::TriplePress:
            return "TriplePress";
        case InputState::TripleRelease:
            return "TripleRelease";
        case InputState::Hold:
            return "Hold";
        case InputState::RawPress:
            return "RawPress";
        case InputState::RawRelease:
            return "RawRelease";
        default:
            return "Unknown";
    }
}

struct InputEvent
{
    InputId    id;
    InputState state;
};

}
