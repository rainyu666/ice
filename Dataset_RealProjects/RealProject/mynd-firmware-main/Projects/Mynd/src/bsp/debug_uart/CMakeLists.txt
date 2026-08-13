set(API_HEADERS
    bsp_debug_uart.h
)

set(SOURCES
    bsp_debug_uart.c
)

target_sources(${projectTarget} PRIVATE ${API_HEADERS} ${SOURCES})

target_include_directories(${projectTarget} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
