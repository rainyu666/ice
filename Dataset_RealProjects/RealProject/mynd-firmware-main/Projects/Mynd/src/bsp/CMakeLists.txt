set(API_HEADERS
    board.h
    board_hw.h
)

set(SOURCES
    board.c
    board_msp.c
)

target_sources(${projectTarget} PRIVATE ${API_HEADERS} ${SOURCES})

target_include_directories(${projectTarget} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

add_subdirectory(adc)
add_subdirectory(bluetooth_uart)
add_subdirectory(debug_uart)
add_subdirectory(shared_i2c)
add_subdirectory(usb_pd_i2c)
