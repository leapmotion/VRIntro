cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-module")
include(Common)

project(VRDemo)

###################################################################################################
# We use an EXTERNAL_LIBRARY_DIR variable as a hint to where all the libraries can be found.
# This is an optional means to not have to specify each library's root dir directly.
###################################################################################################

# find_path(EXTERNAL_LIBRARY_DIR "glew-1.9.0" HINTS /opt/local/Libraries PATHS $ENV{PATH} $ENV{EXTERNAL_LIBRARY_DIR} NO_DEFAULT_PATH)

# TODO: Make EXTERNAL_LIBRARY_DIR detection optional, since users may not have their libraries
# installed the same way we (Leap) do.
# if(EXTERNAL_LIBRARY_DIR STREQUAL "EXTERNAL_LIBRARY_DIR-NOTFOUND")
    # message(STATUS "External Library Directory not found, please specify a folder to look for external libraries")
    # return()
# endif()

# CMAKE_PREFIX_PATH is the path used for searching by FIND_XXX(), with appropriate suffixes added.
# EXTERNAL_LIBRARY_DIR is a hint for all the find_library calls.
# list(INSERT CMAKE_PREFIX_PATH 0 ${EXTERNAL_LIBRARY_DIR})

###################################################################################################
# Definition of how to build the BasicShapes app
###################################################################################################

set(VRDemo_SOURCES
    main.cpp
    ShapesApplication.cpp
    ShapesApplication.h
    SampleListener.cpp
    SampleListener.h
    InteractionLayer.cpp
    InteractionLayer.h
    GridLayer.cpp
    GridLayer.h
    PassthroughLayer.cpp
    PassthroughLayer.h
    QuadsLayer.cpp
    QuadsLayer.h
    SpheresLayer.cpp
    SpheresLayer.h
    SpaceLayer.cpp
    SpaceLayer.h
)

set(_add_executable_options "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # This is the correct way to detect Mac OS X operating system -- see http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    set(_add_executable_options "MACOSX_BUNDLE")
endif()

add_executable(VRDemo ${_add_executable_options} ${VRDemo_SOURCES})

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
target_link_components(VRDemo PUBLIC Application GLController GLShaderLoader GLTexture2Loader Primitives SDLController OculusVR)

target_package(VRDemo Leap LINK_TYPE PUBLIC)
target_package(VRDemo Eigen LINK_TYPE PUBLIC)
