cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-module")

project(VRIntro)

# External module includes
include(ConditionalSources)

# Set up our output directories to be in our root
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# Run all of our project configuration steps
add_subdirectory(source)

# Include directory setup
set(INCLUDE_INSTALL_DIR "${CMAKE_SOURCE_DIR}/source" CACHE PATH "Path to include files for VRIntro")

# Let everyone know we're here
include(CMakePackageConfigHelpers)
configure_package_config_file(
  VRIntroConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/VRIntroConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/VRIntro/cmake
  PATH_VARS INCLUDE_INSTALL_DIR
)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/VRIntroVersion.cmake
                                 VERSION 1.0.0
                                 COMPATIBILITY SameMajorVersion )

# Add all targets to the build-tree export set
export(TARGETS VRIntroLib
  FILE "${PROJECT_BINARY_DIR}/VRIntroTargets.cmake")