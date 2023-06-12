#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.13)
project(libquack)

#-------------------------------------------------------------------------------

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
    cmake_policy(SET CMP0135 NEW)
endif()

#-------------------------------------------------------------------------------

option(LIBQUACK_SHARED "Build shared libquack" OFF)
option(LIBQUACK_SAMPLES "Build libquack samples" ON)

#-------------------------------------------------------------------------------
# FetchContent

include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

set(FREETYPE_URL https://downloads.sourceforge.net/project/freetype/freetype2/2.13.0/freetype-2.13.0.tar.xz)
set(FREETYPE_HASH SHA256=5ee23abd047636c24b2d43c6625dcafc66661d1aca64dec9e0d05df29592624c)

set(HARFBUZZ_URL https://github.com/harfbuzz/harfbuzz/releases/download/7.1.0/harfbuzz-7.1.0.tar.xz)
set(HARFBUZZ_HASH SHA256=f135a61cd464c9ed6bc9823764c188f276c3850a8dc904628de2a87966b7077b)

set(OPENAL_URL https://openal-soft.org/openal-releases/openal-soft-1.22.2.tar.bz2)
set(OPENAL_HASH SHA256=ae94cc95cda76b7cc6e92e38c2531af82148e76d3d88ce996e2928a1ea7c3d20)

set(OGG_URL https://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.gz)
set(OGG_HASH SHA256=0eb4b4b9420a0f51db142ba3f9c64b333f826532dc0f48c6410ae51f4799b664)

set(VORBIS_URL https://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.xz)
set(VORBIS_HASH SHA256=b33cc4934322bcbf6efcbacf49e3ca01aadbea4114ec9589d1b1e9d20f72954b)

#-------------------------------------------------------------------------------
# PkgConfig, may be used to find some dependencies

find_package(PkgConfig)

#-------------------------------------------------------------------------------
# -lm

find_library(MATH_LIBRARY m)

#-------------------------------------------------------------------------------
# Threads

find_package(Threads MODULE REQUIRED)

if(EMSCRIPTEN)
    add_compile_options(-pthread)
    add_link_options(-pthread)
endif()

#-------------------------------------------------------------------------------
# OpenGL & OpenGL ES

find_package(OpenGL MODULE) # OpenGL::{GL,GLX}
find_library(GLES2_LIBRARY GLESv2) # -lGLESv2

#-------------------------------------------------------------------------------
# Xlib & GLX

find_package(X11 MODULE) # X11_INCLUDE_DIR, X11_LIBRARIES

if(X11_FOUND)
    add_library(X11 INTERFACE)
    target_include_directories(X11 INTERFACE ${X11_INCLUDE_DIR})
    target_link_libraries(X11 INTERFACE ${X11_LIBRARIES})
endif()

#-------------------------------------------------------------------------------
# OpenAL

find_package(OpenAL MODULE) # OPENAL_INCLUDE_DIR, OPENAL_LIBRARY

if(OPENAL_FOUND)
    add_library(OpenAL INTERFACE)
    target_include_directories(OpenAL INTERFACE ${OPENAL_INCLUDE_DIR})
    target_link_libraries(OpenAL INTERFACE ${OPENAL_LIBRARY})
else()
    set(ALSOFT_UTILS OFF CACHE INTERNAL "Enable OpenAL utils")
    set(ALSOFT_EXAMPLES OFF CACHE INTERNAL "Enable OpenAL examples")
    set(ALSOFT_BUILD_IMPORT_LIB OFF CACHE INTERNAL "Enable .def/.lib generation")

    if(WIN32)
        set(ALSOFT_BACKEND_PIPEWIRE OFF CACHE INTERNAL "Enable OpenAL PipeWire backend")
    endif()

    FetchContent_Declare(openal URL ${OPENAL_URL} URL_HASH ${OPENAL_HASH})
    FetchContent_MakeAvailable(openal)
endif()

#-------------------------------------------------------------------------------
# FreeType

find_package(Freetype 2.6 MODULE) # Freetype::Freetype

if(NOT FREETYPE_FOUND)
    FetchContent_Declare(freetype URL ${FREETYPE_URL} URL_HASH ${FREETYPE_HASH})
    FetchContent_MakeAvailable(freetype)

    add_library(Freetype::Freetype ALIAS freetype)
endif()

#-------------------------------------------------------------------------------
# harfbuzz

find_package(harfbuzz CONFIG) # harfbuzz::harfbuzz

if(NOT harfbuzz_FOUND)
    if(EMSCRIPTEN)
        add_compile_options(-Wno-cast-function-type-strict)
    endif()

    set(HB_HAVE_FREETYPE ON CACHE INTERNAL "Enable FreeType for Harfbuzz")
    set(HB_BUILD_SUBSET OFF CACHE INTERNAL "Build hb-subset")

    FetchContent_Declare(harfbuzz URL ${HARFBUZZ_URL} URL_HASH ${HARFBUZZ_HASH})
    FetchContent_MakeAvailable(harfbuzz)

    add_library(harfbuzz::harfbuzz ALIAS harfbuzz)
endif()

#-------------------------------------------------------------------------------
# Ogg Vorbis

if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_VORBISFILE vorbisfile vorbis ogg)

    if(PC_VORBISFILE_FOUND)
        add_library(libvorbisfile INTERFACE)
        target_include_directories(libvorbisfile INTERFACE ${PC_VORBISFILE_INCLUDE_DIRS})
        target_link_directories(libvorbisfile INTERFACE ${PC_VORBISFILE_LIBRARY_DIRS})
        target_link_libraries(libvorbisfile INTERFACE ${PC_VORBISFILE_LIBRARIES})

        add_library(vorbis::vorbisfile ALIAS libvorbisfile)
    endif()
endif()

if(NOT TARGET libvorbisfile)
    set(BUILD_TESTING OFF CACHE INTERNAL "Build OGG testing executables")

    FetchContent_Declare(ogg URL ${OGG_URL} URL_HASH ${OGG_HASH})
    FetchContent_MakeAvailable(ogg)

    set(OGG_INCLUDE_DIR ${libogg_SOURCE_DIR}/include ${libogg_BINARY_DIR}/include)
    set(OGG_LIBRARY $<TARGET_FILE:ogg>)

    FetchContent_Declare(vorbis URL ${VORBIS_URL} URL_HASH ${VORBIS_HASH})
    FetchContent_MakeAvailable(vorbis)

    add_library(vorbis::vorbisfile ALIAS vorbisfile)
endif()

#-------------------------------------------------------------------------------

if(LIBQUACK_SHARED)
    set(LIBQUACK_TYPE SHARED)
else()
    set(LIBQUACK_TYPE STATIC)
endif()

add_library(libquack ${LIBQUACK_TYPE}
    src/qu_array.c
    src/qu_audio_null.c
    src/qu_audio_openal.c
    src/qu_core_emscripten.c
    src/qu_core_null.c
    src/qu_core_unix.c
    src/qu_core_win32.c
    src/qu_fs.c
    src/qu_gateway.c
    src/qu_graphics_gl.c
    src/qu_graphics_gles2.c
    src/qu_graphics_null.c
    src/qu_halt.c
    src/qu_image.c
    src/qu_log.c
    src/qu_math.c
    src/qu_sound.c
    src/qu_text.c
    src/qu_thread.c
    src/qu_util.c)

target_compile_options(libquack
    PUBLIC
        $<$<BOOL:${EMSCRIPTEN}>:-sUSE_SDL>)

target_compile_definitions(libquack
    PRIVATE
        $<$<BOOL:${LIBQUACK_SHARED}>:QU_SHARED>
        $<$<BOOL:${WIN32}>:QU_DISABLE_GLES2>
        $<$<OR:$<BOOL:${EMSCRIPTEN}>,$<BOOL:${ANDROID}>>:QU_DISABLE_GL>)

target_include_directories(libquack
    PUBLIC
        include/libqu)

target_link_options(libquack
    PUBLIC
        $<$<BOOL:${EMSCRIPTEN}>:-sINITIAL_MEMORY=838860800>
        $<$<BOOL:${EMSCRIPTEN}>:-sPTHREAD_POOL_SIZE=8>
        $<$<BOOL:${EMSCRIPTEN}>:--shell-file ${CMAKE_SOURCE_DIR}/src/emscripten-shell.html>)

target_link_libraries(libquack
    PUBLIC
        $<$<BOOL:${MATH_LIBRARY}>:${MATH_LIBRARY}>
    PRIVATE
        Threads::Threads
        $<TARGET_NAME_IF_EXISTS:OpenGL::GL>
        $<TARGET_NAME_IF_EXISTS:OpenGL::GLX>
        $<$<BOOL:${GLES2_LIBRARY}>:${GLES2_LIBRARY}>
        $<TARGET_NAME_IF_EXISTS:X11>
        $<TARGET_NAME_IF_EXISTS:OpenAL>
        $<$<BOOL:${WIN32}>:XInput>
        Freetype::Freetype
        harfbuzz::harfbuzz
        vorbis::vorbisfile)

set_target_properties(libquack PROPERTIES
    PREFIX ""
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS ON
    C_VISIBILITY_PRESET hidden)

#-------------------------------------------------------------------------------
# Non-MSVC stuff

if(NOT MSVC)
    target_compile_options(libquack
        PUBLIC
            -Wall
            -Wextra
            -Wno-switch
            -Wno-switch-default
            -Wno-unused-parameter
            -Wno-unused-variable)

    if(NOT WIN32)
        target_compile_options(libquack PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
        target_link_libraries(libquack PUBLIC $<$<CONFIG:Debug>:-fsanitize=address>)
    endif()
endif()

#-------------------------------------------------------------------------------

if(LIBQUACK_SAMPLES)
    add_subdirectory(samples)
endif()

#-------------------------------------------------------------------------------
