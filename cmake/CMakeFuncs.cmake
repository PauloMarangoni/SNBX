function(add_binary_file folder name)
    add_custom_target("${name}_bin" ALL DEPENDS "${CMAKE_BINARY_DIR}/bin/${name}")

    if (WIN32)
        set(ARCH_FOLDER_NAME "win64")
        add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/${name}
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${folder}/bin/${ARCH_FOLDER_NAME}/${name}.dll
                ${CMAKE_BINARY_DIR}/bin/${name}.dll
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${folder}/bin/${ARCH_FOLDER_NAME}/${name}.dll)
    elseif (UNIX AND NOT APPLE)
        set(ARCH_FOLDER_NAME "linux64")
        add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/${name}
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${folder}/bin/${ARCH_FOLDER_NAME}/lib${name}.so
                ${CMAKE_BINARY_DIR}/bin/lib${name}.so
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${folder}/bin/${ARCH_FOLDER_NAME}/lib${name}.so)
    elseif (APPLE)
        set(ARCH_FOLDER_NAME "macOS")
        add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/${name}
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${folder}/bin/${ARCH_FOLDER_NAME}/lib${name}.dylib
                ${CMAKE_BINARY_DIR}/bin/lib${name}.dylib
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${folder}/bin/${ARCH_FOLDER_NAME}/lib${name}.dylib)
    endif ()
endfunction(add_binary_file folder name)