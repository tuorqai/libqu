#-------------------------------------------------------------------------------

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    add_link_options(--preload-file ${CMAKE_SOURCE_DIR}/samples/assets@assets)
endif()

add_executable(01_base 01_base.c)
target_link_libraries(01_base libquack)

add_executable(02_loop 02_loop.c)
target_link_libraries(02_loop libquack)

add_executable(03_primitives 03_primitives.c)
target_link_libraries(03_primitives libquack)

add_executable(04_textures 04_textures.c)
target_link_libraries(04_textures libquack)

add_executable(05_input 05_input.c)
target_link_libraries(05_input libquack)

add_executable(06_sounds 06_sounds.c)
target_link_libraries(06_sounds libquack)

add_executable(07_canvas 07_canvas.c)
target_link_libraries(07_canvas libquack)

#-------------------------------------------------------------------------------
