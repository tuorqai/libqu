
add_library(libqu
    "qu_array.c"
    "qu_audio_null.c"
    "qu_audio_openal.c"
    "qu_core_emscripten.c"
    "qu_core_null.c"
    "qu_core_unix.c"
    "qu_core_win32.c"
    "qu_fs.c"
    "qu_gateway.c"
    "qu_graphics_gl2.c"
    "qu_graphics_gles2.c"
    "qu_graphics_null.c"
    "qu_halt.c"
    "qu_image.c"
    "qu_log.c"
    "qu_math.c"
    "qu_platform_posix.c"
    "qu_platform_win32.c"
    "qu_sound.c"
    "qu_text.c"
    "qu_util.c")

target_include_directories(libqu
    PUBLIC "../include/libqu"
    PRIVATE "../include")

target_compile_options(libqu
    PUBLIC
        $<$<BOOL:${EMSCRIPTEN}>:-sUSE_SDL>)

target_compile_definitions(libqu
    PRIVATE
        QU_BUILD
        $<$<BOOL:${BUILD_SHARED_LIBS}>:QU_SHARED>
        $<$<BOOL:${WIN32}>:QU_DISABLE_GLES2>
        $<$<OR:$<BOOL:${EMSCRIPTEN}>,$<BOOL:${ANDROID}>>:QU_DISABLE_GL>)

target_link_options(libqu
    PUBLIC
        $<$<BOOL:${EMSCRIPTEN}>:-sINITIAL_MEMORY=838860800>
        $<$<BOOL:${EMSCRIPTEN}>:-sPTHREAD_POOL_SIZE=8>
        $<$<BOOL:${EMSCRIPTEN}>:--shell-file ${CMAKE_SOURCE_DIR}/src/emscripten-shell.html>)

target_link_libraries(libqu
    PUBLIC
        $<$<BOOL:${MATH_LIBRARY}>:${MATH_LIBRARY}>
    PRIVATE
        Threads::Threads
        $<TARGET_NAME_IF_EXISTS:OpenGL::GL>
        $<TARGET_NAME_IF_EXISTS:OpenGL::GLX>
        $<$<BOOL:${GLES2_LIBRARY}>:${GLES2_LIBRARY}>
        $<TARGET_NAME_IF_EXISTS:X11>
        $<TARGET_NAME_IF_EXISTS:OpenAL::OpenAL>
        $<$<BOOL:${WIN32}>:dwmapi shcore XInput>
        Freetype::Freetype
        harfbuzz::harfbuzz
        Vorbis::vorbisfile)

set_target_properties(libqu
    PROPERTIES
        PREFIX ""
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
        C_VISIBILITY_PRESET hidden)

if(NOT MSVC)
    target_compile_options(libqu
        PUBLIC
            -Wall
            -Wextra
            -Wno-switch
            -Wno-switch-default
            -Wno-unused-parameter
            -Wno-unused-variable)

    if(NOT WIN32)
        target_compile_options(libqu PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
        target_link_libraries(libqu PUBLIC $<$<CONFIG:Debug>:-fsanitize=address>)
    endif()
endif()
