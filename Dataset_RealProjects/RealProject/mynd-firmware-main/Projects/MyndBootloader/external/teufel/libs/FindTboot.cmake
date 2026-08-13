if (NOT TeufelLibsPath)
    set(Tboot_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/teufel/libs/t-boot)
else()
    set(Tboot_PATH ${TeufelLibsPath}/t-boot)
endif()

if(NOT (TARGET Tboot))
    add_library(Tboot INTERFACE IMPORTED)
    target_compile_definitions(Tboot INTERFACE "-DT_BOOT")
    target_sources(Tboot INTERFACE
        "${Tboot_PATH}/src/bootloader/dfu/t_boot_dfu.c"
        "${Tboot_PATH}/src/bootloader/dfu/t_boot_dfu.h"
        "${Tboot_PATH}/src/bootloader/encryption/t_boot_encryption.c"
        "${Tboot_PATH}/src/bootloader/encryption/t_boot_encryption.h")
    target_include_directories(Tboot INTERFACE "${Tboot_PATH}/src/bootloader/dfu")
    target_include_directories(Tboot INTERFACE "${Tboot_PATH}/src/bootloader/encryption")
    target_include_directories(Tboot INTERFACE "${Tboot_PATH}/src/logger_config")
endif()

if(NOT (TARGET Tboot::RamDisk))
    add_library(Tboot::RamDisk INTERFACE IMPORTED)
    target_sources(Tboot::RamDisk INTERFACE
            "${Tboot_PATH}/src/bootloader/ram_disk/t_boot_ram_disk.c"
            "${Tboot_PATH}/src/bootloader/ram_disk/t_boot_ram_disk.h")
    target_include_directories(Tboot::RamDisk INTERFACE "${Tboot_PATH}/src/bootloader/ram_disk")
    target_link_libraries(Tboot::RamDisk INTERFACE Tboot)
endif()

if(NOT (TARGET Tboot::Platform::Stm32))
    add_library(Tboot::Platform::Stm32 INTERFACE IMPORTED)
    target_include_directories(Tboot::Platform::Stm32 INTERFACE "${Tboot_PATH}/src/device_specific/stm32f0")
    target_link_libraries(Tboot::Platform::Stm32 INTERFACE Tboot)
endif()

if(NOT (TARGET Tboot::Platform::Stm32::Crc))
    add_library(Tboot::Platform::Stm32::Crc INTERFACE IMPORTED)
    target_include_directories(Tboot::Platform::Stm32::Crc INTERFACE "${Tboot_PATH}/src/device_specific/stm32f0/crc")
    target_link_libraries(Tboot::Platform::Stm32::Crc INTERFACE Tboot::Platform::Stm32)
    target_sources(Tboot::Platform::Stm32::Crc INTERFACE
        "${Tboot_PATH}/src/device_specific/stm32f0/crc/t_boot_crc.c"
        "${Tboot_PATH}/src/device_specific/stm32f0/crc/t_boot_crc.h")
endif()

if(NOT (TARGET Tboot::Platform::Stm32::Usb))
    add_library(Tboot::Platform::Stm32::Usb INTERFACE IMPORTED)
    target_include_directories(Tboot::Platform::Stm32::Usb INTERFACE "${Tboot_PATH}/src/device_specific/stm32f0/usbd")
    target_link_libraries(Tboot::Platform::Stm32::Usb INTERFACE Tboot::Platform::Stm32)
    file(GLOB TBOOT_STM32_USB_SRCS_SRCS ${Tboot_PATH}/src/device_specific/stm32f0/usbd/*.c
                                        ${Tboot_PATH}/src/device_specific/stm32f0/usbd/*.h)
    target_sources(Tboot::Platform::Stm32::Usb INTERFACE ${TBOOT_STM32_USB_SRCS_SRCS})
endif()

if(NOT (TARGET Tboot::Platform::RT1010))
    add_library(Tboot::Platform::RT1010 INTERFACE IMPORTED)
    target_include_directories(Tboot::Platform::RT1010 INTERFACE "${Tboot_PATH}/src/device_specific/mimxrt1010")
    target_link_libraries(Tboot::Platform::RT1010 INTERFACE Tboot)
endif()

if(NOT (TARGET Tboot::Platform::RT1010::Crc))
    add_library(Tboot::Platform::RT1010::Crc INTERFACE IMPORTED)
    target_include_directories(Tboot::Platform::RT1010::Crc INTERFACE "${Tboot_PATH}/src/device_specific/mimxrt1010/crc")
    target_link_libraries(Tboot::Platform::RT1010::Crc INTERFACE Tboot::Platform::RT1010)
    target_sources(Tboot::Platform::RT1010::Crc INTERFACE
        "${Tboot_PATH}/src/device_specific/mimxrt1010/crc/t_boot_crc.c"
        "${Tboot_PATH}/src/device_specific/mimxrt1010/crc/t_boot_crc.h")
endif()

if(NOT (TARGET Tboot::Platform::Agnostic))
    add_library(Tboot::Platform::Agnostic INTERFACE IMPORTED)
    target_include_directories(Tboot::Platform::Agnostic INTERFACE "${Tboot_PATH}/src/device_specific/agnostic")
    target_link_libraries(Tboot::Platform::Agnostic INTERFACE Tboot)
endif()

if(NOT (TARGET Tboot::Platform::Agnostic::Crc))
    add_library(Tboot::Platform::Agnostic::Crc INTERFACE IMPORTED)
    target_include_directories(Tboot::Platform::Agnostic::Crc INTERFACE "${Tboot_PATH}/src/device_specific/agnostic/crc")
    target_link_libraries(Tboot::Platform::Agnostic::Crc INTERFACE Tboot::Platform::Agnostic)
    target_sources(Tboot::Platform::Agnostic::Crc INTERFACE
            "${Tboot_PATH}/src/device_specific/agnostic/crc/t_boot_crc.c"
            "${Tboot_PATH}/src/device_specific/agnostic/crc/t_boot_crc.h"
            "${Tboot_PATH}/src/device_specific/agnostic/crc/crc32.c"
            "${Tboot_PATH}/src/device_specific/agnostic/crc/crc32.h")
endif()

if(NOT (TARGET Tboot::Tests))
    add_library(Tboot::Tests INTERFACE IMPORTED)
    target_include_directories(Tboot::Tests INTERFACE "${Tboot_PATH}/tests")
    target_sources(Tboot::Tests INTERFACE
            "${Tboot_PATH}/tests/test_parse_dfu_packets.c"
            "${Tboot_PATH}/tests/test_generate_crc.c")
    target_link_libraries(Tboot::Tests INTERFACE Tboot)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Tboot
    FOUND_VAR Tboot_FOUND
    REQUIRED_VARS Tboot_PATH
    HANDLE_COMPONENTS
)
