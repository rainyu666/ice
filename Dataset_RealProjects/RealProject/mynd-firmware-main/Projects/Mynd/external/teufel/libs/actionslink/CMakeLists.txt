cmake_minimum_required(VERSION 3.13)

set(ACTIONSLINK_VERSION "0.2.0")

######################################
######### Cortex-m7 platform #########
######################################
project(actionslink-cortex-m7 LANGUAGES ASM C)

project(actionslink-cortex-m7 VERSION ${ACTIONSLINK_VERSION})

configure_file (
    "${CMAKE_SOURCE_DIR}/middleware/actionslink/actionslink_version.h.in"
    "${PROJECT_BINARY_DIR}/actionslink_version.h"
)

include(gcc_mimxrt1010)

find_package(TEUFEL COMPONENTS cbuf circ_batch_buf circ_contiguous_buf REQUIRED)

# Nanopb
set(NANOPB_SRC_ROOT_FOLDER ${CMAKE_SOURCE_DIR}/middleware/nanopb/nanopb-0.4.4)
find_package(Nanopb REQUIRED)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/middleware/actionslink/proto/app_base.proto)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/middleware/actionslink/proto/audio.proto)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/middleware/actionslink/proto/bluetooth.proto)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/middleware/actionslink/proto/common.proto)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/middleware/actionslink/proto/hfp.proto)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/middleware/actionslink/proto/gaia.proto)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/middleware/actionslink/proto/mgmt.proto)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/middleware/actionslink/proto/pio.proto)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/middleware/actionslink/proto/power.proto)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/middleware/actionslink/proto/transport.proto)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/middleware/actionslink/proto/upgrade.proto)
NANOPB_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${CMAKE_SOURCE_DIR}/middleware/actionslink/proto/usb.proto)

aux_source_directory(${CMAKE_SOURCE_DIR}/middleware/actionslink ACTIONSLINK_SOURCES)

set(ALL_SOURCES
    ${TEUFEL_SOURCES}
    ${ACTIONSLINK_SOURCES}
)

add_library(actionslink-cortex-m7 STATIC
    ${ALL_SOURCES}
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

# Public API
target_include_directories(actionslink-cortex-m7 PUBLIC
    ${CMAKE_SOURCE_DIR}/middleware/actionslink
)

set(PUBLIC_HDRS
    actionslink.h
    actionslink_log.h
)

target_include_directories(actionslink-cortex-m7 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NANOPB_INCLUDE_DIRS}

    # .proto files generated into bin folder
    ${CMAKE_BINARY_DIR}/middleware/actionslink
    ${PROJECT_BINARY_BIN}
)

set_target_properties(actionslink-cortex-m7 PROPERTIES PUBLIC_HEADER "${PUBLIC_HDRS}")

MIMXRT1010_SET_TARGET_PROPERTIES(actionslink-cortex-m7)

install(TARGETS actionslink-cortex-m7
    ARCHIVE DESTINATION actionslink-cortex-m7/lib
    PUBLIC_HEADER DESTINATION actionslink-cortex-m7/include
)

include(stm32_gcc)

######################################
######### Cortex-m4 platform #########
######################################
find_package(CMSIS COMPONENTS STM32L4 REQUIRED)

add_library(actionslink-cortex-m4 STATIC
    ${ALL_SOURCES}
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_include_directories(actionslink-cortex-m4 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NANOPB_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/middleware/actionslink

    # .proto files generated into bin folder
    ${CMAKE_BINARY_DIR}/middleware/actionslink
)

target_link_libraries(actionslink-cortex-m4 PRIVATE
    CMSIS::STM32::L432KC
    STM32::NoSys
)

set_target_properties(actionslink-cortex-m4 PROPERTIES PUBLIC_HEADER "${PUBLIC_HDRS}")

install(TARGETS actionslink-cortex-m4
    ARCHIVE DESTINATION actionslink-cortex-m4/lib
    PUBLIC_HEADER DESTINATION actionslink-cortex-m4/include
)

######################################
######### Cortex-m0 platform #########
######################################
find_package(CMSIS COMPONENTS STM32F0 REQUIRED)

add_library(actionslink-cortex-m0 STATIC
    ${ALL_SOURCES}
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_include_directories(actionslink-cortex-m0 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NANOPB_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/middleware/actionslink

    # .proto files generated into bin folder
    ${CMAKE_BINARY_DIR}/middleware/actionslink
)

target_link_libraries(actionslink-cortex-m0 PRIVATE
    CMSIS::STM32::F030RC
    STM32::NoSys
)

set_target_properties(actionslink-cortex-m0 PROPERTIES PUBLIC_HEADER "${PUBLIC_HDRS}")

install(TARGETS actionslink-cortex-m0
    ARCHIVE DESTINATION actionslink-cortex-m0/lib
    PUBLIC_HEADER DESTINATION actionslink-cortex-m0/include
)

