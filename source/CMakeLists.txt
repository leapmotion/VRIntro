cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-module")
include(Common)

project(VRIntro)

set(VRIntro_SOURCES
    main.cpp
    VRIntroApp.cpp
    VRIntroApp.h
    LeapListener.cpp
    LeapListener.h
    InteractionLayer.cpp
    InteractionLayer.h
    PassthroughLayer.cpp
    PassthroughLayer.h
    HandLayer.cpp
    HandLayer.h
    MessageLayer.cpp
    MessageLayer.h
    GridLayer.cpp
    GridLayer.h
    SpheresLayer.cpp
    SpheresLayer.h
    SpaceLayer.cpp
    SpaceLayer.h
    FlyingLayer.cpp
    FlyingLayer.h
    LifeLayer.cpp
    LifeLayer.h
    FractalLayer.cpp
    FractalLayer.h
    QuadsLayer.cpp
    QuadsLayer.h
    PlatformInitializer.h
    PrecisionTimer.h
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(VRIntro_SOURCES
        ${VRIntro_SOURCES}
        PlatformInitializerMac.mm
        PlatformInitializerMac.h
    )
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    set(VRIntro_SOURCES
        ${VRIntro_SOURCES}
        PlatformInitializerWin.cpp
        PlatformInitializerWin.h
        Mirror.h
        Mirror.cpp
    )
endif()

ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" VRIntro_SOURCES)

option(SHOW_CONSOLE "Display console window for debugging" OFF)

set(_add_executable_options "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # This is the correct way to detect Mac OS X operating system -- see http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    set(_add_executable_options "MACOSX_BUNDLE")
endif()

if(NOT SHOW_CONSOLE AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    set(_add_executable_options WIN32)
endif()

add_executable(VRIntro ${_add_executable_options} ${VRIntro_SOURCES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # This is the correct way to detect Mac OS X operating system -- see http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall -Werror")
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_DEBUG_FLAGS} /Zi /fp:fast /MP")
    set(CMAKE_CXX_RELEASE_FLAGS "${CMAKE_CXX_RELEASE_FLAGS} /fp:except /MP")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT /ignore:4099")
endif()

# The definition of Components::* targets and automatic finding and linking of all library
# dependencies is all accomplished via the ComponentsConfig.cmake file (in the root build dir
# or root install dir of the Components package).
find_package(Components)
# The target_link_components function finds and links exactly the minimal set of libraries
# needed by the specified components.
target_link_components(VRIntro Application GLController GLShaderLoader GLTexture2Loader Primitives SDLController OculusVR)

target_package(VRIntro Leap)
target_package(VRIntro Eigen)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  target_link_libraries(VRIntro comctl32.lib Dwmapi.lib)
endif()

###################################################################################################
# Resource rules
###################################################################################################

include(DefinePostBuildResourceCopyRules)
define_post_build_resource_copy_rules(
    TARGET
        VRIntro
    RELATIVE_PATH_RESOURCES
        shaders/fractal-frag.glsl
        shaders/fractal-vert.glsl
        shaders/passthrough-frag.glsl
        shaders/passthrough-vert.glsl
        shaders/solid-frag.glsl
        shaders/solid-vert.glsl
        shaders/transparent-frag.glsl
        shaders/transparent-vert.glsl
        images/help.png
        images/lowfps.png
        images/no_oculus.png
        images/level3_popup.png
        images/level4_popup.png
        images/no_images.png
        images/random.png
        RunMirrored.bat
    ABSOLUTE_PATH_RESOURCES
        ${Components_DIR}/resources/material-frag.glsl
        ${Components_DIR}/resources/matrix-transformed-vert.glsl
)
