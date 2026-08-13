SET(DRIVERS_COMPONENTS aw9523b bq25713 button STM32_vEEPROM tas5805m tas5825p tps25751)


set(DRIVERS_PICKED_COMPONENTS "")
set(DRIVERS_PREFIX "")
set(DRIVERS_PATH ${CMAKE_CURRENT_LIST_DIR})

if(NOT ${TeufelDrivers_REQUIRED_COMPONENTS})
    MESSAGE(FATAL_ERROR "Unknown DRIVERS component: empty list")
endif()

# Check if these components supported
foreach(cmp ${TeufelDrivers_FIND_COMPONENTS})
    if(NOT (${cmp} IN_LIST DRIVERS_COMPONENTS))
        MESSAGE(FATAL_ERROR "Unknown DRIVERS component: ${cmp}. Available components: ${DRIVERS_COMPONENTS}")
    endif()
endforeach()

# Add all components into desired list of components
foreach(cmp ${TeufelDrivers_FIND_COMPONENTS})
    if(NOT (${cmp} IN_LIST DRIVERS_PICKED_COMPONENTS))
        LIST(APPEND DRIVERS_PICKED_COMPONENTS ${cmp})
    endif()
endforeach()

set(TeufelDrivers_SOURCES "")
set(TeufelDrivers_INCLUDE_DIR "")

# Add basic include
list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH})

if("aw9523b" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/aw9523b/aw9523b.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/aw9523b)
endif()

if("bq25713" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/bq25713/bq25713.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/bq25713)
endif()

if("button" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/input_handlers/button/button_handler.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/input_handlers/input_events.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/input_handlers)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/input_handlers/button)
    if (TEUFEL_TESTS_ENABLED)
        file(GLOB BUTTON_TESTS ${DRIVERS_PATH}/input_handlers/tests/*.c)
        list(APPEND TeufelDrivers_SOURCES ${BUTTON_TESTS})
        list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/input_handlers/tests)
    endif()
endif()

if("STM32_vEEPROM" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/STM32_vEEPROM/eeprom.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/STM32_vEEPROM/virtual_eeprom.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/STM32_vEEPROM)
endif()

if("tas5805m" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/tas5805m/tas5805m.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/tasxxxx_volume_table/tasxxxx_volume_table.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/tas5805m)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/tasxxxx_volume_table)
endif()

if("tas5825p" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/tas5825p/tas5825p.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/tasxxxx_volume_table/tasxxxx_volume_table.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/tas5825p)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/tasxxxx_volume_table)
endif()

if("tps25751" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/tps25751/tps25751.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/tps25751)
endif()


include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(TeufelDrivers DEFAULT_MSG TeufelDrivers_INCLUDE_DIR TeufelDrivers_SOURCES)
