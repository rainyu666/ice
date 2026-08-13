SET(DRIVERS_COMPONENTS ad82128 adau145x adau1701 aw9523b bq25713 bq24610 bq78350 button CY8CMBRXXXX dwam83 ep91a6s
                       ep91a7e ep9xax ep950e es8156 es9018k2m et6416y ew21s ht1635 ia9qh5 is31fl3206 is31fl323xa
                       is31fl3196 is31fl3216 is31fl3236 ir_nec mapx opt3001 pcm1681 pcm1690 pcm186x RT1010_vEEPROM
                       STM32_vEEPROM spi-flash ssd1322 ssd1362 swa sunplus tas5805m tas5825p tps25750 touch_slider
                       tm1680)

                      
                      


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

if("ad82128" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/ad82128/ad82128.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/ad82128)
endif()


if("adau145x" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/adau145x/adau145x.c)
#    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/adau170x/SigmaStudioFW.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/adau145x)
endif()

if("adau1701" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/adau170x/adau1701.c)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/adau170x/SigmaStudioFW.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/adau170x)
endif()

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

if("touch_slider" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/input_handlers/touch_slider/touch_slider_handler.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/input_handlers/input_events.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/input_handlers)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/input_handlers/touch_slider)
endif()

if("bq24610" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/bq24610/bq24610.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/bq24610)
endif()

if("bq78350" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/bq78350/bq78350.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/bq78350)
endif()

if("CY8CMBRXXXX" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/CY8CMBRXXXX/CY8CMBR3xxx_APIs.c)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/CY8CMBRXXXX/CY8CMBR3xxx_CRC.c)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/CY8CMBRXXXX/CY8CMBR3xxx.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/CY8CMBRXXXX)
endif()

if("dwam83" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/dwam83/dwam83.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/dwam83)
endif()

if("ep91a6s" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/ep91a6s/ep91a6s.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/ep91a6s)
endif()

if("ep91a7e" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/ep91a7e/ep91a7e.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/ep91a7e)
endif()

if("ep9xax" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/ep9xax/ep9xax.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/ep9xax)
endif()

if("ep950e" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/ep950e/ep950e.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/ep950e)
endif()

if("es9018k2m" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/es9018k2m/es9018k2m.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/es9018k2m)
endif()

if("ew21s" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/ew21s/ew21s.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/ew21s)
endif()

if("es8156" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/es8156/es8156.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/es8156)
endif()

if("et6416y" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/et6416y/et6416y.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/et6416y)
endif()

if("ht1635" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/ht1635/ht1635.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/ht1635/display_ht1635.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/ht1635)
endif()

if("ir_nec" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/ir_nec/ir_nec.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/ir_nec)
endif()

if("is31fl323xa" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/is31fl323xa/is31fl323xa.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/is31fl323xa)
endif()

if("is31fl3196" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/is31fl3196/is31fl3196.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/is31fl3196)
endif()

if("is31fl3216" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/is31fl3216/is31fl3216.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/is31fl3216)
endif()

if("is31fl3236" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/is31fl3236/is31fl3236.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/is31fl3236)
endif()

if("is31fl3206" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/is31fl3206/is31fl3206.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/is31fl3206)
endif()

if("mapx" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/mapx/mapx.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/mapx)
endif()

if("RT1010_vEEPROM" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/RT1010_vEEPROM/eeprom.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/RT1010_vEEPROM/virtual_eeprom.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/RT1010_vEEPROM)
endif()

if("opt3001" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/opt3001/opt3001.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/opt3001)
endif()

if("pcm1681" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/pcm1681/pcm1681.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/pcm1681)
endif()

if("pcm1690" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/pcm1690/pcm1690.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/pcm1690)
endif()

if("pcm186x" IN_LIST DRIVERS_PICKED_COMPONENTS)
    LIST(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/pcm186x/pcm186x.c)
    LIST(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/pcm186x)
endif()


if("STM32_vEEPROM" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/STM32_vEEPROM/eeprom.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/STM32_vEEPROM/virtual_eeprom.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/STM32_vEEPROM)
endif()

if("spi-flash" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/spi-flash/spi-flash.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/spi-flash)
endif()

if("ssd1362" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/ssd1362/ssd1362.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/ssd1362)
endif()

if("ssd1322" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/ssd1322/ssd1322.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/ssd1322)
endif()

if("sunplus" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/sunplus/commands/sunplus_commands.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/sunplus/driver/sunplus_driver.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/sunplus/protocol/sunplus_protocol_ll.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/sunplus/protocol/sunplus_protocol_ul.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/sunplus/utils/sunplus_utils.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/sunplus)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/sunplus/commands)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/sunplus/driver)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/sunplus/protocol)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/sunplus/utils)
endif()

if("swa" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/swa/swa.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/swa)
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

if("tps25750" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/tps25750/tps25750.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/tps25750)
endif()

if("tm1680" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/tm1680/tm1680.c)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/tm1680/display_tm1680.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/tm1680)
endif()

if("ia9qh5" IN_LIST DRIVERS_PICKED_COMPONENTS)
    list(APPEND TeufelDrivers_SOURCES ${DRIVERS_PATH}/ia9qh5/ia9qh5.c)
    list(APPEND TeufelDrivers_INCLUDE_DIR ${DRIVERS_PATH}/ia9qh5)
endif()

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(TeufelDrivers DEFAULT_MSG TeufelDrivers_INCLUDE_DIR TeufelDrivers_SOURCES)
