#pragma once
#include "tas5805m.h"

// From 2024-07-26

// TAS5805M from normal-Mode to ECO-Mode (set volume to -3dB)
const tas5805m_cfg_reg_t tas5805m_normal_to_eco_mode_config_1[] = {
    { 0x00, 0x00 },     // Page 0
    { 0x7f, 0x8C },     // Book 0x8C
    { 0x00, 0x2A },     // Page 0x2A
    { CFG_META_BURST, 9 },
    { 0x24, 0x00 },
    { 0x5A, 0x9D },
    { 0xF8, 0x00 },
    { 0x5A, 0x9D },
    { 0xF8, 0x00 }
};

// TAS5805M from normal-Mode to ECO-Mode (set DRC settings)
const tas5805m_cfg_reg_t tas5805m_normal_to_eco_mode_config_2[] = {
    { 0x00, 0x00 },     // Page 0
    { 0x7f, 0x8C },     // Book 0x8C
    { 0x00, 0x2B },     // Page 0x2B
    { CFG_META_BURST, 41 },
    { 0x34, 0x00 }, //   DRC Settings of band2:   attack: 1 ms, release: 100 ms, energy: 1 ms, Region1 - Threshold: -124 dB, Offset: 0 dB, Ratio: 1   Region2 - Threshold: -60 dB, Offset: 0 dB, Ratio: 1   Region3 - Threshold: 0 dB, Offset: 0 dB, Ratio: 100
    { 0x88, 0x3f },
    { 0xd1, 0x00 },
    { 0x88, 0x3f },
    { 0xd1, 0x00 },
    { 0x06, 0xd3 },
    { 0x72, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0xff },
    { 0xcb, 0x56 },
    { 0x4f, 0xff },
    { 0x81, 0x47 },
    { 0xae, 0xfb },
    { 0x99, 0x33 },
    { 0xb7, 0xfd },
    { 0x0d, 0x42 },
    { 0x09, 0xff },
    { 0xd9, 0xbb },
    { 0x3c, 0xff },
    { 0x40, 0xa8 },
    { 0x2e, 0x00 },
};


// TAS5805M from ECO-Mode to normal-Mode (set volume to 1dB)
const tas5805m_cfg_reg_t tas5805m_eco_to_normal_mode_config_1[] = {
    { 0x00, 0x00 },     // Page 0
    { 0x7f, 0x8C },     // Book 0x8C
    { 0x00, 0x2A },     // Page 0x2A
    { CFG_META_BURST, 9 },
    { 0x24, 0x00 },
    { 0x8f, 0x9e },
    { 0x4d, 0x00 },
    { 0x8f, 0x9e },
    { 0x4d, 0x00 }
};

// TAS5805M from ECO-Mode to normal-Mode (set DRC settings)
const tas5805m_cfg_reg_t tas5805m_eco_to_normal_mode_config_2[] = {
    { 0x00, 0x00 },     // Page 0
    { 0x7f, 0x8C },     // Book 0x8C
    { 0x00, 0x2B },     // Page 0x2B
    { CFG_META_BURST, 41 },
    { 0x34, 0x00 }, //   DRC Settings of band2:   attack: 1 ms, release: 100 ms, energy: 1 ms, Region1 - Threshold: -124 dB, Offset: 0 dB, Ratio: 1   Region2 - Threshold: -60 dB, Offset: 0 dB, Ratio: 1   Region3 - Threshold: 0 dB, Offset: 0 dB, Ratio: 100
    { 0x88, 0x3f },
    { 0xd1, 0x00 },
    { 0x88, 0x3f },
    { 0xd1, 0x00 },
    { 0x06, 0xd3 },
    { 0x72, 0x00 },
    { 0x00, 0x00 },
    { 0x00, 0xff },
    { 0xd0, 0xbd },
    { 0x0c, 0xff },
    { 0x81, 0x47 },
    { 0xae, 0xfb },
    { 0x99, 0x33 },
    { 0xb7, 0xfd },
    { 0x37, 0xc7 },
    { 0x54, 0xff },
    { 0xd9, 0xbb },
    { 0x3c, 0xff },
    { 0x40, 0xa8 },
    { 0x2e, 0x00 },
};

#define TAS5805M_NORMAL_TO_ECO_MODE_CONFIG1_REGISTERS_SIZE (sizeof(tas5805m_normal_to_eco_mode_config_1) / sizeof(tas5805m_normal_to_eco_mode_config_1[0]))
#define TAS5805M_NORMAL_TO_ECO_MODE_CONFIG2_REGISTERS_SIZE (sizeof(tas5805m_normal_to_eco_mode_config_2) / sizeof(tas5805m_normal_to_eco_mode_config_2[0]))
#define TAS5805M_ECO_TO_NORMAL_MODE_CONFIG1_REGISTERS_SIZE (sizeof(tas5805m_eco_to_normal_mode_config_1) / sizeof(tas5805m_eco_to_normal_mode_config_1[0]))
#define TAS5805M_ECO_TO_NORMAL_MODE_CONFIG2_REGISTERS_SIZE (sizeof(tas5805m_eco_to_normal_mode_config_2) / sizeof(tas5805m_eco_to_normal_mode_config_2[0]))
