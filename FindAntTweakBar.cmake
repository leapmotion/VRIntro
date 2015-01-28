#.rst
# FindAntTweakBar
# ------------
#
# Created by Chen Zheng.
# Locate and configure AntTweakBar
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   AntTweakBar::AntTweakBar
#
# Variables
# ^^^^^^^^^
#   AntTweakBar_ROOT_DIR
#   AntTweakBar_FOUND
#   AntTweakBar_INCLUDE_DIR
#   AntTweakBar_SHARED_LIB
#   AntTweakBar_IMPORT_LIB - Windows only
#

find_path(AntTweakBar_ROOT_DIR
          NAMES include/AntTweakBar.h
          PATH_SUFFIXES AntTweakBar${ALTERNATE_LIBRARY}
                        AntTweakBar
)

set(AntTweakBar_INCLUDE_DIR ${AntTweakBar_ROOT_DIR}/include)

if(MSVC)
  find_library(AntTweakBar_IMPORT_LIB NAMES AntTweakBar.lib AntTweakBar64.lib HINTS ${AntTweakBar_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(AntTweakBar_IMPORT_LIB)
endif()

list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 ${CMAKE_SHARED_LIBRARY_SUFFIX})  
find_library(AntTweakBar_SHARED_LIB NAMES AntTweakBar AntTweakBar64 HINTS ${AntTweakBar_ROOT_DIR} PATH_SUFFIXES lib)
list(REMOVE_AT CMAKE_FIND_LIBRARY_SUFFIXES 0)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AntTweakBar DEFAULT_MSG
                                  AntTweakBar_ROOT_DIR AntTweakBar_INCLUDE_DIR
                                  AntTweakBar_SHARED_LIB)

include(CreateImportTargetHelpers)
generate_import_target(AntTweakBar SHARED)

