
set(EXECUTABLE "hello-surfaces")

add_executable(${EXECUTABLE} main.c)
target_link_libraries(${EXECUTABLE} libqu)

if(WIN32 AND BUILD_SHARED_LIBS AND (CMAKE_VERSION VERSION_GREATER_EQUAL 3.21))
    add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${EXECUTABLE}>
            $<TARGET_FILE_DIR:${EXECUTABLE}>
        COMMAND_EXPAND_LISTS)
endif()