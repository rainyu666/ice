set(API_HEADERS
    e_config.h
    kvstorage.h
    eeprom_config.h
)

set(SOURCES
    e_config.c
)

target_sources(${projectTarget} PRIVATE ${API_HEADERS} ${SOURCES})

target_include_directories(${projectTarget} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
