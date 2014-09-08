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
    QuadsLayer.cpp
    QuadsLayer.h
)

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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT /DEBUG")
endif()

# The definition of Components::* targets and automatic finding and linking of all library
# dependencies is all accomplished via the ComponentsConfig.cmake file (in the root build dir
# or root install dir of the Components package).
find_package(Components)
# The target_link_components function finds and links exactly the minimal set of libraries
# needed by the specified components.
target_link_components(VRIntro PUBLIC Application GLController GLShaderLoader GLTexture2Loader Primitives SDLController OculusVR)

target_package(VRIntro Leap LINK_TYPE PUBLIC)
target_package(VRIntro Eigen LINK_TYPE PUBLIC)

###################################################################################################
# Resource rules
###################################################################################################

set(RESOURCE_FILES
  shaders/lighting-frag.glsl
  shaders/lighting-vert.glsl
  shaders/passthrough-frag.glsl
  shaders/passthrough-vert.glsl
  shaders/solid-frag.glsl
  shaders/solid-vert.glsl
  shaders/transparent-frag.glsl
  shaders/transparent-vert.glsl
  help.png
  lowfps.png
  no_oculus.png
)

# TODO: install steps (or resource-copying steps) for other platforms
# TODO: really it would be good to make a cmake module for how to install files in a cross-platform way.
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # This is the correct way to detect Mac OS X operating system -- see http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    set(ACTUAL_BUILD_DIR "${PROJECT_BINARY_DIR}")
    if(${CMAKE_GENERATOR} MATCHES "Xcode")
        # CMAKE_BUILD_TYPE will be one of Release, Debug, etc.
        set(ACTUAL_BUILD_DIR "${ACTUAL_BUILD_DIR}/${CMAKE_BUILD_TYPE}")
    endif()
    # TODO: apparently there is a different "correct" way to install files on Mac;
    # see: http://www.cmake.org/cmake/help/v3.0/prop_sf/MACOSX_PACKAGE_LOCATION.html
    # http://www.cmake.org/cmake/help/v3.0/prop_tgt/RESOURCE.html
    # http://www.cmake.org/Wiki/CMake:Bundles_And_Frameworks#Bundle_only
    install(FILES ${RESOURCE_FILES} DESTINATION "${ACTUAL_BUILD_DIR}/VRIntro.app/Contents/Resources")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # TODO: figure out how this should be done correctly -- do we want to use the POSIX directory scheme, or what?
    # For now, just install into the binary dir, because that's where the SDL_GetBasePath will point us to.
    install(FILES ${RESOURCE_FILES} DESTINATION ${PROJECT_BINARY_DIR})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    install(FILES ${RESOURCE_FILES} DESTINATION "${PROJECT_BINARY_DIR}/Release")
    install(FILES ${RESOURCE_FILES} DESTINATION "${PROJECT_BINARY_DIR}/Debug")
endif()

