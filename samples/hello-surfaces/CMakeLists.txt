
set(EXECUTABLE "hello-surfaces")

add_executable(${EXECUTABLE} main.c)
target_link_libraries(${EXECUTABLE} libqu)

if(WIN32)
    if(LIBQUACK_SHARED)
        add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:libquack> $<TARGET_FILE_DIR:${EXECUTABLE}>)
    endif()

    if(TARGET OpenAL)
        add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:OpenAL> $<TARGET_FILE_DIR:${EXECUTABLE}>)
    endif()
endif()
