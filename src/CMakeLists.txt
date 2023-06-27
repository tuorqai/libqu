
set(TARGET_NAME libqu)

add_library(${TARGET_NAME}
    "qu.h"
    "qu_array.c"
    "qu_audio_null.c"
    "qu_core_null.c"
    "qu_fs.c"
    "qu_gateway.c"
    "qu_graphics_null.c"
    "qu_halt.c"
    "qu_image.c"
    "qu_log.c"
    "qu_math.c"
    "qu_sound.c"
    "qu_text.c"
    "qu_util.c")

target_include_directories(${TARGET_NAME}
    PUBLIC "../include/libqu"
    PRIVATE "../include")

target_compile_definitions(${TARGET_NAME}
    PRIVATE
        QU_BUILD
        $<$<BOOL:${BUILD_SHARED_LIBS}>:QU_SHARED>)

target_link_libraries(${TARGET_NAME}
    PUBLIC
        $<$<BOOL:${MATH_LIBRARY}>:${MATH_LIBRARY}>
    PRIVATE
        Threads::Threads
        Freetype::Freetype
        harfbuzz::harfbuzz
        Vorbis::vorbisfile)

set_target_properties(${TARGET_NAME}
    PROPERTIES
        PREFIX ""
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
        C_VISIBILITY_PRESET hidden)

if(EMSCIPTEN)
    target_sources(${TARGET_NAME}
        PRIVATE
            "qu_core_emscripten.c"
            "qu_platform_posix.c")

    target_compile_options(${TARGET_NAME}
        PUBLIC
            -sUSE_SDL)

    target_link_options(${TARGET_NAME}
        PUBLIC
            -sINITIAL_MEMORY=838860800
            -sPTHREAD_POOL_SIZE=8
            --shell-file "${CMAKE_SOURCE_DIR}/src/emscripten-shell.html")
elseif(WIN32)
    target_sources(${TARGET_NAME}
        PRIVATE
            "qu_core_win32.c"
            "qu_platform_win32.c")

    target_link_libraries(${TARGET_NAME}
        PRIVATE
            dwmapi
            shcore
            XInput)
elseif(UNIX)
    target_sources(${TARGET_NAME}
        PRIVATE
            "qu_platform_posix.c")

    if(TARGET X11::X11)
        target_sources(${TARGET_NAME}
            PRIVATE
                "qu_core_unix.c") # rename to x11

        target_link_libraries(${TARGET_NAME}
            PRIVATE
                OpenGL::GLX
                X11::X11)
    endif()
endif()

if(TARGET OpenGL::GL)
    target_sources(${TARGET_NAME} PRIVATE "qu_graphics_gl2.c")
    target_compile_definitions(${TARGET_NAME} PRIVATE QU_USE_GL)
    target_link_libraries(${TARGET_NAME} PRIVATE OpenGL::GL)
endif()

if(GLES2_LIBRARY)
    target_sources(${TARGET_NAME} PRIVATE "qu_graphics_gles2.c")
    target_compile_definitions(${TARGET_NAME} PRIVATE QU_USE_GLES2)
    target_link_libraries(${TARGET_NAME} PRIVATE ${GLES2_LIBRARY})
endif()

if(TARGET OpenAL::OpenAL)
    target_sources(${TARGET_NAME} PRIVATE "qu_audio_openal.c")
    target_compile_definitions(${TARGET_NAME} PRIVATE QU_USE_OPENAL)
    target_link_libraries(${TARGET_NAME} PRIVATE OpenAL::OpenAL)
endif()

if(MSVC)
    target_compile_definitions(${TARGET_NAME}
        PRIVATE
            _CRT_SECURE_NO_WARNINGS)

    target_compile_options(${TARGET_NAME}
        PRIVATE
            /W4
            /wd4100
            /wd4152
            /wd4244)

    if(QU_USE_ASAN)
        target_compile_options(${TARGET_NAME} PRIVATE /fsanitize=address)
    endif()
else()
    target_compile_options(${TARGET_NAME}
        PRIVATE
            -Wall
            -Wextra
            -Wno-switch
            -Wno-switch-default
            -Wno-unused-parameter
            -Wno-unused-variable)

    if(QU_USE_ASAN)
        target_compile_options(${TARGET_NAME} PRIVATE -fsanitize=address)
        target_link_libraries(${TARGET_NAME} PUBLIC -fsanitize=address)
    endif()
endif()
