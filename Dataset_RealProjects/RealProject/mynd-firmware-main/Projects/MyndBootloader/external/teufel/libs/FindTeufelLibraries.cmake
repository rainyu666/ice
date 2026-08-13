SET(LIBRARIES_COMPONENTS app_assert audio buffer cbuf circ_batch_buf circ_contiguous_buf cli core_utils crc8
         dwt_profiler greeting hashmap_string llist menu power property sysaudio syscalls tshell util)


set(LIBRARIES_PICKED_COMPONENTS "")
set(LIBRARIES_PREFIX "")
set(LIBRARIES_PATH ${CMAKE_CURRENT_LIST_DIR})

if(NOT ${TeufelLibraries_REQUIRED_COMPONENTS})
    MESSAGE(FATAL_ERROR "Unknown LIBRARIES component: empty list")
endif()

# Check if these components supported
foreach(cmp ${TeufelLibraries_FIND_COMPONENTS})
    if(NOT (${cmp} IN_LIST LIBRARIES_COMPONENTS))
        MESSAGE(FATAL_ERROR "Unknown LIBRARIES component: ${cmp}. Available components: ${LIBRARIES_COMPONENTS}")
    endif()
endforeach()

# Add all components into desired list of components
foreach(cmp ${TeufelLibraries_FIND_COMPONENTS})
    if(NOT (${cmp} IN_LIST LIBRARIES_PICKED_COMPONENTS))
        LIST(APPEND LIBRARIES_PICKED_COMPONENTS ${cmp})
    endif()
endforeach()

set(TeufelLibraries_SOURCES "")
set(TeufelLibraries_INCLUDE_DIR "")

# Add basic include
list(APPEND TeufelLibraries_INCLUDE_DIR ${LIBRARIES_PATH})

if("app_assert" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/app_assert/app_assert.c)
endif()

if("buffer" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/buffer/buffer.c)
endif()

if("cbuf" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/cbuf/cbuf.c)
endif()

if("circ_batch_buf" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/circ_batch_buf/circ_batch_buf.c)
endif()

if("circ_contiguous_buf" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/circ_contiguous_buf/circ_contiguous_buf.c)
endif()


if("crc8" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/crc8/crc8.c)
endif()

if("dwt_profiler" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/dwt_profiler/dwt_profiler.c)
endif()

if("hashmap_string" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/hashmap_string/hashmap_string.c)
endif()

if("greeting" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/greeting/greeting.c)
endif()

if("llist" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/llist/llist.c)
endif()

if("menu" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/menu/menu.c)
endif()

if("sysaudio" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/sysaudio/sysaudio.c)
endif()

if("syscalls" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/syscalls/syscalls.c)
endif()

if("tshell" IN_LIST LIBRARIES_PICKED_COMPONENTS)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/tshell/tshell.c)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/tshell/tshell_args_parser.c)
    list(APPEND TeufelLibraries_SOURCES ${LIBRARIES_PATH}/tshell/tshell_printf.c)
endif()

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(TeufelLibraries DEFAULT_MSG TeufelLibraries_INCLUDE_DIR TeufelLibraries_SOURCES)
