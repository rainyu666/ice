if (NOT TeufelLibsPath)
    set(Actionslink_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/teufel/libs/actionslink)
else()
    set(Actionslink_PATH ${TeufelLibsPath}/actionslink)
endif()

if(NOT (TARGET Actionslink))
    add_library(Actionslink INTERFACE IMPORTED)
    target_sources(Actionslink INTERFACE
            "${Actionslink_PATH}/src/api/actionslink.c"
            "${Actionslink_PATH}/src/api/actionslink.h"
            "${Actionslink_PATH}/src/api/actionslink_types.h"
            "${Actionslink_PATH}/src/decoders/actionslink_decoders.c"
            "${Actionslink_PATH}/src/decoders/actionslink_decoders.h"
            "${Actionslink_PATH}/src/encoders/actionslink_encoders.c"
            "${Actionslink_PATH}/src/encoders/actionslink_encoders.h"
            "${Actionslink_PATH}/src/events/actionslink_events.c"
            "${Actionslink_PATH}/src/events/actionslink_events.h"
            "${Actionslink_PATH}/src/requests/actionslink_requests.c"
            "${Actionslink_PATH}/src/requests/actionslink_requests.h"
            "${Actionslink_PATH}/src/log/actionslink_log.c"
            "${Actionslink_PATH}/src/log/actionslink_log.h"
            "${Actionslink_PATH}/src/transport/actionslink_bt_ll.c"
            "${Actionslink_PATH}/src/transport/actionslink_bt_ll.h"
            "${Actionslink_PATH}/src/transport/actionslink_bt_ul.c"
            "${Actionslink_PATH}/src/transport/actionslink_bt_ul.h"
            "${Actionslink_PATH}/src/utils/actionslink_utils.c"
            "${Actionslink_PATH}/src/utils/actionslink_utils.h")
    target_include_directories(Actionslink INTERFACE "${Actionslink_PATH}/src/api")
    target_include_directories(Actionslink INTERFACE "${Actionslink_PATH}/src/decoders")
    target_include_directories(Actionslink INTERFACE "${Actionslink_PATH}/src/encoders")
    target_include_directories(Actionslink INTERFACE "${Actionslink_PATH}/src/events")
    target_include_directories(Actionslink INTERFACE "${Actionslink_PATH}/src/requests")
    target_include_directories(Actionslink INTERFACE "${Actionslink_PATH}/src/log")
    target_include_directories(Actionslink INTERFACE "${Actionslink_PATH}/src/transport")
    target_include_directories(Actionslink INTERFACE "${Actionslink_PATH}/src/utils")
endif()

if(NOT (TARGET Actionslink::LogLevelOff))
    add_library(Actionslink::LogLevelOff INTERFACE IMPORTED)
    target_compile_definitions(Actionslink::LogLevelOff INTERFACE "-DACTIONSLINK_LOG_LEVEL=0")
    target_link_libraries(Actionslink::LogLevelOff INTERFACE Actionslink)
endif()

if(NOT (TARGET Actionslink::LogLevelError))
    add_library(Actionslink::LogLevelError INTERFACE IMPORTED)
    target_compile_definitions(Actionslink::LogLevelError INTERFACE "-DACTIONSLINK_LOG_LEVEL=1")
    target_link_libraries(Actionslink::LogLevelError INTERFACE Actionslink)
endif()

if(NOT (TARGET Actionslink::LogLevelWarning))
    add_library(Actionslink::LogLevelWarning INTERFACE IMPORTED)
    target_compile_definitions(Actionslink::LogLevelWarning INTERFACE "-DACTIONSLINK_LOG_LEVEL=2")
    target_link_libraries(Actionslink::LogLevelWarning INTERFACE Actionslink)
endif()

if(NOT (TARGET Actionslink::LogLevelInfo))
    add_library(Actionslink::LogLevelInfo INTERFACE IMPORTED)
    target_compile_definitions(Actionslink::LogLevelInfo INTERFACE "-DACTIONSLINK_LOG_LEVEL=3")
    target_link_libraries(Actionslink::LogLevelInfo INTERFACE Actionslink)
endif()

if(NOT (TARGET Actionslink::LogLevelDebug))
    add_library(Actionslink::LogLevelDebug INTERFACE IMPORTED)
    target_compile_definitions(Actionslink::LogLevelDebug INTERFACE "-DACTIONSLINK_LOG_LEVEL=4")
    target_link_libraries(Actionslink::LogLevelDebug INTERFACE Actionslink)
endif()

if(NOT (TARGET Actionslink::LogLevelTrace))
    add_library(Actionslink::LogLevelTrace INTERFACE IMPORTED)
    target_compile_definitions(Actionslink::LogLevelTrace INTERFACE "-DACTIONSLINK_LOG_LEVEL=5")
    target_link_libraries(Actionslink::LogLevelTrace INTERFACE Actionslink)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Actionslink
        FOUND_VAR Actionslink_FOUND
        REQUIRED_VARS Actionslink_PATH
        HANDLE_COMPONENTS
)
