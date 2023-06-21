
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.14...3.26)

cmake_policy(SET CMP0048 NEW)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
    cmake_policy(SET CMP0135 NEW)
endif()

project(libqu
    VERSION 0.1
    DESCRIPTION "2D game library written in C99"
    LANGUAGES C)

#-------------------------------------------------------------------------------

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(LIBQU_BUILD_SAMPLES "Build libqu samples" ON)

#-------------------------------------------------------------------------------
# CPM

set(CPM_DOWNLOAD_VERSION 0.38.1)

if(CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
        https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION})
endif()

include(${CPM_DOWNLOAD_LOCATION})

#-----------------------------------------------------------
# -lm

find_library(MATH_LIBRARY m)

#-----------------------------------------------------------
# Threads

find_package(Threads MODULE REQUIRED)

if(EMSCRIPTEN)
    add_compile_options(-pthread)
    add_link_options(-pthread)
endif()

#-----------------------------------------------------------
# OpenGL & OpenGL ES

find_package(OpenGL MODULE) # OpenGL::{GL,GLX}
find_library(GLES2_LIBRARY GLESv2) # -lGLESv2

#-----------------------------------------------------------
# Xlib & GLX

find_package(X11 MODULE) # X11_INCLUDE_DIR, X11_LIBRARIES

if(X11_FOUND)
    add_library(X11 INTERFACE)
    target_include_directories(X11 INTERFACE ${X11_INCLUDE_DIR})
    target_link_libraries(X11 INTERFACE ${X11_LIBRARIES})
endif()

#-----------------------------------------------------------
# OpenAL-soft

CPMAddPackage(NAME OpenAL
    VERSION 1.22.2
    OPTIONS
        "ALSOFT_UTILS OFF"
        "ALSOFT_EXAMPLES OFF"
        "ALSOFT_BUILD_IMPORT_LIB OFF"
    URL https://openal-soft.org/openal-releases/openal-soft-1.22.2.tar.bz2
    URL_HASH SHA256=ae94cc95cda76b7cc6e92e38c2531af82148e76d3d88ce996e2928a1ea7c3d20)

#-----------------------------------------------------------
# FreeType

CPMAddPackage(NAME freetype
    VERSION 2.13
    OPTIONS
        "FT_DISABLE_BZIP2 ON"
        "FT_DISABLE_HARFBUZZ ON"
        "FT_DISABLE_BROTLI ON"
    URL https://downloads.sourceforge.net/project/freetype/freetype2/2.13.0/freetype-2.13.0.tar.xz
    URL_HASH SHA256=5ee23abd047636c24b2d43c6625dcafc66661d1aca64dec9e0d05df29592624c)

if(freetype_ADDED)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

#-----------------------------------------------------------
# harfbuzz

CPMAddPackage(NAME harfbuzz
    VERSION 7.1.0
    OPTIONS
        "HB_HAVE_FREETYPE ON"
        "HB_BUILD_SUBSET OFF"
    URL https://github.com/harfbuzz/harfbuzz/releases/download/7.1.0/harfbuzz-7.1.0.tar.xz
    URL_HASH SHA256=f135a61cd464c9ed6bc9823764c188f276c3850a8dc904628de2a87966b7077b)

if(harfbuzz_ADDED)
    if(EMSCRIPTEN)
        add_compile_options(-Wno-cast-function-type-strict)
    endif()

    add_library(harfbuzz::harfbuzz ALIAS harfbuzz)
endif()

#-----------------------------------------------------------
# xiph/ogg

CPMAddPackage(NAME ogg
    VERSION 1.3.5
    OPTIONS
        "BUILD_TESTING OFF"
    URL http://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.gz
    URL_HASH SHA256=0eb4b4b9420a0f51db142ba3f9c64b333f826532dc0f48c6410ae51f4799b664)

#-----------------------------------------------------------
# xiph/vorbis

CPMAddPackage(NAME vorbis
    VERSION 1.3.7
    OPTIONS
        "OGG_INCLUDE_DIR ${ogg_SOURCE_DIR}/include ${ogg_BINARY_DIR}/include"
        "OGG_LIBRARY $<TARGET_FILE:ogg>"
    URL http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.xz
    URL_HASH SHA256=b33cc4934322bcbf6efcbacf49e3ca01aadbea4114ec9589d1b1e9d20f72954b)

if(vorbis_ADDED)
    add_library(Vorbis::vorbis ALIAS vorbis)
    add_library(Vorbis::vorbisenc ALIAS vorbisenc)
    add_library(Vorbis::vorbisfile ALIAS vorbisfile)
endif()

#-------------------------------------------------------------------------------

add_subdirectory(src)

#-------------------------------------------------------------------------------

if(LIBQU_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

#-------------------------------------------------------------------------------

include(GNUInstallDirs)

install(TARGETS libqu)
install(DIRECTORY "include/libqu" TYPE INCLUDE)
