if (NOT TeufelLibsPath)
    set(Logger_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/teufel/libs/logger)
else()
    set(Logger_PATH ${TeufelLibsPath}/logger)
endif()

find_path(Logger_COMMON_INCLUDE
    NAMES "logger.h"
    PATHS "${Logger_PATH}"
    CMAKE_FIND_ROOT_PATH_BOTH
)
list(APPEND Logger_INCLUDE_DIRS "${Logger_COMMON_INCLUDE}")

if(NOT (TARGET Logger))
    add_library(Logger INTERFACE IMPORTED)
    target_compile_definitions(Logger INTERFACE "-DTEUFEL_LOGGER")
    target_include_directories(Logger INTERFACE "${Logger_PATH}")
    target_sources(Logger INTERFACE
        "${Logger_PATH}/logger.h"
        "${Logger_PATH}/logger.c"
        "${Logger_PATH}/implementations/logger_weak_implementation.c"
    )
endif()

if(NOT (TARGET Logger::Syscalls))
    add_library(Logger::Syscalls INTERFACE IMPORTED)
    target_sources(Logger::Syscalls INTERFACE "${Logger_PATH}/logger_syscalls.c")
    target_link_libraries(Logger::Syscalls INTERFACE Logger)
endif()

###### Pre-defined Configs ######

if(NOT (TARGET Logger::Config1))
    add_library(Logger::Config1 INTERFACE IMPORTED)
    target_include_directories(Logger::Config1 INTERFACE "${Logger_PATH}/configs/num1")
    target_link_libraries(Logger::Config1 INTERFACE Logger)
endif()

if(NOT (TARGET Logger::Config2))
    add_library(Logger::Config2 INTERFACE IMPORTED)
    target_include_directories(Logger::Config2 INTERFACE "${Logger_PATH}/configs/num2")
    target_link_libraries(Logger::Config2 INTERFACE Logger)
endif()

if(NOT (TARGET Logger::ConfigOff))
    add_library(Logger::ConfigOff INTERFACE IMPORTED)
    target_include_directories(Logger::ConfigOff INTERFACE "${Logger_PATH}/configs/off")
    target_link_libraries(Logger::ConfigOff INTERFACE Logger)
endif()

###### Pre-defined Formats ######

if(NOT (TARGET Logger::Format1))
    add_library(Logger::Format1 INTERFACE IMPORTED)
    target_include_directories(Logger::Format1 INTERFACE "${Logger_PATH}/formats/num1")
    target_link_libraries(Logger::Format1 INTERFACE Logger)
endif()

if(NOT (TARGET Logger::Format2))
    add_library(Logger::Format2 INTERFACE IMPORTED)
    target_include_directories(Logger::Format2 INTERFACE "${Logger_PATH}/formats/num2")
    target_link_libraries(Logger::Format2 INTERFACE Logger)
endif()

if(NOT (TARGET Logger::Format3))
    add_library(Logger::Format3 INTERFACE IMPORTED)
    target_include_directories(Logger::Format3 INTERFACE "${Logger_PATH}/formats/num3")
    target_link_libraries(Logger::Format3 INTERFACE Logger)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Logger
    REQUIRED_VARS Logger_INCLUDE_DIRS
    FOUND_VAR Logger_FOUND
    HANDLE_COMPONENTS
)
