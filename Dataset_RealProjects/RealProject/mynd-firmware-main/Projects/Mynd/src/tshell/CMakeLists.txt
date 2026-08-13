set(API_HEADERS
    tshell.h
    tshell_args_parser.h
    tshell_printf.h
)

set(SOURCES
    tshell.c
    tshell_args_parser.c
    tshell_printf.c
)

target_sources(${projectTarget} PRIVATE ${API_HEADERS} ${SOURCES})

target_include_directories(${projectTarget} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
