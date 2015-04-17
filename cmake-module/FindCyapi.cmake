#.rst
# FindCyAPI
# ------------
#
# Created by Walter Gray.
# Locate and configure CyAPI
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   CyAPI::CyAPI
#
# Variables
# ^^^^^^^^^
#  CyAPI_ROOT_DIR
#  CyAPI_FOUND
#  CyAPI_INCLUDE_DIR
#  CyAPI_LIBRARY


find_path(CyAPI_ROOT_DIR
          NAMES inc/CyAPI.h
          PATH_SUFFIXES CyAPI-${CyAPI_FIND_VERSION}
                        CyAPI
)

set(CyAPI_INCLUDE_DIR ${CyAPI_ROOT_DIR}/inc)

if(BUILD_64_BIT)
  set(ARCH_FOLDER x64)
else()
  set(ARCH_FOLDER x86)
endif()

find_library(CyAPI_LIBRARY_DEBUG "CyAPI.lib" HINTS ${CyAPI_ROOT_DIR}/lib/${ARCH_FOLDER}/Debug)
find_library(CyAPI_LIBRARY_RELEASE "CyAPI.lib" HINTS ${CyAPI_ROOT_DIR}/lib/${ARCH_FOLDER}/Release)

include(SelectConfigurations)
select_configurations(CyAPI LIBRARY LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CyAPI DEFAULT_MSG CyAPI_INCLUDE_DIR CyAPI_LIBRARIES)

include(CreateImportTargetHelpers)
generate_import_target(CyAPI STATIC)
