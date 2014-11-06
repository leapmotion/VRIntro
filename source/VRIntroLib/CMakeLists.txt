option(USE_BULLET "Use Bullet physics engine to build block simulation level" OFF)

CONFIGURE_FILE(Config.h.in ${PROJECT_BINARY_DIR}/Config.h @ONLY IMMEDIATE)
include_directories(${PROJECT_BINARY_DIR})

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
  MathUtility.h
  MathUtility.cpp
  PhysicsLayer.h
)

add_windows_sources(
  VRIntroLib_SOURCES
  PlatformInitializerWin.cpp
  PlatformInitializerWin.h
  Mirror.h
  Mirror.cpp
)

if(USE_BULLET)
  set(VRIntroLib_SOURCES ${VRIntroLib_SOURCES} PhysicsLayer.cpp)
endif()

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
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall -Werror -Wno-reorder -Wno-unused-variable -Wno-unused-private-field")
endif()

if(NOT SHOW_CONSOLE AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    set(_add_executable_options WIN32)
endif()

add_library(VRIntroLib STATIC ${VRIntroLib_SOURCES})

target_link_components(VRIntroLib Application GLController GLShaderLoader GLTexture2Loader Primitives SDLController OculusVR)
target_package(VRIntroLib Eigen)
if(USE_BULLET)
  target_package(VRIntroLib Bullet)
endif()

if(WIN32)
  target_link_libraries(VRIntroLib comctl32.lib Dwmapi.lib Shlwapi.lib)
endif()
