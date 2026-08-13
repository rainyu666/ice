if (NOT TeufelLibsPath)
    set(IEngine_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/teufel/libs/IEngine)
else()
    set(IEngine_PATH ${TeufelLibsPath}/IEngine)
endif()

if(NOT IEngine_PATH)
    message(FATAL "No IEngine_PATH specified using default: ${IEngine_PATH}")
endif()

find_path(IEngine_COMMON_INCLUDE
    NAMES "IndicationEngine.h"
    PATHS "${IEngine_PATH}"
    CMAKE_FIND_ROOT_PATH_BOTH
)
list(APPEND IEngine_INCLUDE_DIRS "${IEngine_COMMON_INCLUDE}")

if(NOT (TARGET IEngine))
    add_library(IEngine INTERFACE IMPORTED)
    target_include_directories(IEngine INTERFACE "${IEngine_COMMON_INCLUDE}")
    target_compile_definitions(IEngine INTERFACE "-DIEngine")
endif()

if(NOT (TARGET IEngine::Pattern::Generic))
    add_library(IEngine::Pattern::Generic INTERFACE IMPORTED)
    target_include_directories(IEngine::Pattern::Generic INTERFACE "${IEngine_COMMON_INCLUDE}/pattern/generic")
    target_link_libraries(IEngine::Pattern::Generic INTERFACE IEngine)
endif()

if(NOT (TARGET IEngine::Pattern::DoubleBlink))
    add_library(IEngine::Pattern::DoubleBlink INTERFACE IMPORTED)
    target_include_directories(IEngine::Pattern::DoubleBlink INTERFACE "${IEngine_COMMON_INCLUDE}/pattern/dblink")
    target_link_libraries(IEngine::Pattern::DoubleBlink INTERFACE IEngine)
endif()

if(NOT (TARGET IEngine::Pattern::Slide))
    add_library(IEngine::Pattern::Slide INTERFACE IMPORTED)
    target_include_directories(IEngine::Pattern::Slide INTERFACE "${IEngine_COMMON_INCLUDE}/pattern/slide")
    target_link_libraries(IEngine::Pattern::Slide INTERFACE IEngine)
endif()

if(NOT (TARGET IEngine::Pattern::Fade))
    add_library(IEngine::Pattern::Fade INTERFACE IMPORTED)
    target_include_directories(IEngine::Pattern::Fade INTERFACE "${IEngine_COMMON_INCLUDE}/pattern/fade")
    target_link_libraries(IEngine::Pattern::Fade INTERFACE IEngine)
endif()

if(NOT (TARGET IEngine::Tests))
    add_library(IEngine::Tests INTERFACE IMPORTED)
    target_sources(IEngine::Tests INTERFACE "${IEngine_PATH}/tests/indication_test.cpp")
    target_sources(IEngine::Tests INTERFACE "${IEngine_PATH}/tests/fade_test.cpp")
    target_link_libraries(IEngine::Tests INTERFACE IEngine)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IEngine
    REQUIRED_VARS IEngine_INCLUDE_DIRS
    FOUND_VAR IEngine_FOUND
    HANDLE_COMPONENTS
)

