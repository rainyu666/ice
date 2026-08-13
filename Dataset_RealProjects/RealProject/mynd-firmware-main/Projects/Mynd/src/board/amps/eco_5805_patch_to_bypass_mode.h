#pragma once
#include "tas5805m.h"

// From 2023-11-01

const tas5805m_cfg_reg_t tas5805m_bypass_config_registers[] = {
    { 0x00, 0x00 },
    { 0x7f, 0x00 },
    { 0x00, 0x00 },
    { 0x66, 0x87 },
};

#define TAS5805M_BYPASS_CONFIG_REGISTERS_SIZE (sizeof(tas5805m_bypass_config_registers) / sizeof(tas5805m_bypass_config_registers[0]))
