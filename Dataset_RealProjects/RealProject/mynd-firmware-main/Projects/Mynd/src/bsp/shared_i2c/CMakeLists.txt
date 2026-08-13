set(API_HEADERS
    bsp_shared_i2c.h
)

set(SOURCES
    bsp_shared_i2c.c
)

target_sources(${projectTarget} PRIVATE ${API_HEADERS} ${SOURCES})

target_include_directories(${projectTarget} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
