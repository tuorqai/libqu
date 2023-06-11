
set(GAME_NAME "surrounded")

add_executable(${GAME_NAME} main.c)
target_link_libraries(${GAME_NAME} libquack)

if(EMSCRIPTEN)
    target_link_options(${GAME_NAME} PRIVATE
        "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets"
        "SHELL:--exclude-file ${CMAKE_CURRENT_SOURCE_DIR}/assets/29.wav"
        "SHELL:--exclude-file ${CMAKE_CURRENT_SOURCE_DIR}/assets/balda.wav")
endif()

add_custom_command(TARGET ${GAME_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${GAME_NAME}>/assets)
