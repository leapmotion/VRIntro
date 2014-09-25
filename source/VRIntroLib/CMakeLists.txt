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

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(VRIntroLib_SOURCES
        ${VRIntroLib_SOURCES}
        PlatformInitializerMac.mm
        PlatformInitializerMac.h
    )
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    set(VRIntroLib_SOURCES
        ${VRIntroLib_SOURCES}
        PlatformInitializerWin.cpp
        PlatformInitializerWin.h
        Mirror.h
        Mirror.cpp
    )
endif()

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
