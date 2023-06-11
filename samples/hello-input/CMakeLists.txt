
add_executable(hello-input main.c)
target_link_libraries(hello-input libquack)

if(EMSCRIPTEN)
    target_link_options(hello-input PRIVATE
        "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets")
endif()

add_custom_command(TARGET hello-input POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:hello-input>/assets)
