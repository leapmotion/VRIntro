include(Common)

set(VRIntroLib_SOURCES
    VRIntroApp.cpp
    VRIntroApp.h
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
    IFrameSupplier.h
)

add_windows_sources(
  VRIntroLib_SOURCES
  PlatformInitializerWin.cpp
  PlatformInitializerWin.h
  Mirror.h
  Mirror.cpp
)

add_mac_sources(
  VRIntroLib_SOURCES
  PlatformInitializerMac.mm
  PlatformInitializerMac.h
)

ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" VRIntroLib_SOURCES)

option(SHOW_CONSOLE "Display console window for debugging" OFF)

set(_add_executable_options "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # This is the correct way to detect Mac OS X operating system -- see http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    set(_add_executable_options "MACOSX_BUNDLE")
endif()

if(NOT SHOW_CONSOLE AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    set(_add_executable_options WIN32)
endif()

add_library(VRIntroLib STATIC ${VRIntroLib_SOURCES})
target_link_components(VRIntroLib Application GLController GLShaderLoader GLTexture2Loader Primitives SDLController OculusVR)
target_package(VRIntroLib Eigen)

if(WIN32)
  target_link_libraries(VRIntroLib comctl32.lib Dwmapi.lib)
endif()


###################################################################################################
# Resource rules
###################################################################################################

include(DefinePostBuildResourceCopyRules)
define_post_build_resource_copy_rules(
    TARGET
        VRIntroLib
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
    ABSOLUTE_PATH_RESOURCES
        ${Components_DIR}/resources/material-frag.glsl
        ${Components_DIR}/resources/matrix-transformed-vert.glsl
)
