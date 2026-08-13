function(add_deploy_jlink TARGET)
    add_custom_target(${TARGET}-deploy-jlink DEPENDS ${TARGET}.hex
        COMMAND JLinkExe -NoGui 1 -CommanderScript ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.JLinkScript
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Deploy firmware via JLink."
        VERBATIM
    )
endfunction()
