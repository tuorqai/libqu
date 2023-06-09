
set(EXECUTABLE "hello-textures")

add_executable(${EXECUTABLE} main.c)
target_link_libraries(${EXECUTABLE} libqu)

if(EMSCRIPTEN)
    target_link_options(${EXECUTABLE} PRIVATE
        "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets")
else()
    add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${EXECUTABLE}>/assets)

    if(WIN32 AND BUILD_SHARED_LIBS AND (CMAKE_VERSION VERSION_GREATER_EQUAL 3.21))
        add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${EXECUTABLE}>
                $<TARGET_FILE_DIR:${EXECUTABLE}>
            COMMAND_EXPAND_LISTS)
    endif()
endif()
