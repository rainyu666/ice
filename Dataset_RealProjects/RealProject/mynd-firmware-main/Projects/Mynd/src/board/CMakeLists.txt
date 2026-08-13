set(API_HEADERS
    board_link.h
)

target_sources(${projectTarget} PRIVATE ${API_HEADERS})

target_include_directories(${projectTarget} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

add_subdirectory(amps)
add_subdirectory(bluetooth)
add_subdirectory(boost_converter)
add_subdirectory(charger)
add_subdirectory(eeprom)
add_subdirectory(hw_revision)
add_subdirectory(io_expander)
add_subdirectory(moisture_detection)
add_subdirectory(plug_detection)
add_subdirectory(power_supply)
add_subdirectory(usb_pd_controller)
add_subdirectory(usb_switch)

