# MYND bootloader changelog

## [Unreleased]

## [2.1.0] - 2024-07-24
### Changed
- Virtual EEPROM flash size

### Added
- Write magic number on UPDATE_STATE_COMPLETE event to signal application to power on after a successful update

## [2.0.0] - 2024-06-14
### Changed
- Flash memory layout

## [1.0.0] - 2024-05-15
### Changed
- Product name (MYND)
- USB pid to the value 0x003F
- Flash memory layout

## [0.2.0] - 2024-03-06
### Added
- Support for HW with extra USB multiplexer for USB PD controller IC

## [0.1.0] - 2024-02-20
### Added
- MCU FW update capability via USB
- System reset when power button is pressed (only if update is not in progress)
- Simple LED indications
