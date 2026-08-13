set(API_HEADERS
    board_link_usb_pd_controller.h
)

set(SOURCES
    board_link_usb_pd_controller.c
)

target_sources(${projectTarget} PRIVATE ${API_HEADERS} ${SOURCES})

target_include_directories(${projectTarget} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
