set(API_HEADERS
    bsp_bluetooth_uart.h
)

set(SOURCES
    bsp_bluetooth_uart.c
)

target_sources(${projectTarget} PRIVATE ${API_HEADERS} ${SOURCES})

target_include_directories(${projectTarget} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
