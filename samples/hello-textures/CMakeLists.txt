
add_executable(hello-textures main.c)
target_link_libraries(hello-textures libquack)

if(EMSCRIPTEN)
    target_link_options(hello-textures PRIVATE
        "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets")
endif()

add_custom_command(TARGET hello-textures POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:hello-textures>/assets)
