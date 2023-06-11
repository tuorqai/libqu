
add_executable(hello-audio main.c)
target_link_libraries(hello-audio libquack)

if(EMSCRIPTEN)
    target_link_options(hello-audio PRIVATE
        "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets")
endif()

add_custom_command(TARGET hello-audio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:hello-audio>/assets)
