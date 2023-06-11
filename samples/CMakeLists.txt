
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

add_subdirectory(hello-libqu)
add_subdirectory(hello-textures)
add_subdirectory(hello-surfaces)
add_subdirectory(hello-input)
add_subdirectory(hello-audio)
add_subdirectory(surrounded)
