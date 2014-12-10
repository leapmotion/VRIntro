#.rst
# FindHalide
# ------------
#
# Created by Walter Gray
# Locate and configure Halide.
# On win32, it will be configured as a dynamic library since that's how it's
# distributed.
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   Halide::Halide
#
# Variables
# ^^^^^^^^^
#   Halide_ROOT_DIR
#   Halide_FOUND
#   Halide_INCLUDE_DIR
#   Halide_LIBRARY
#   Halide_STATIC_LIB
#   Halide_SHARED_LIB
#   Halide_IMPORT_LIB
#   Halide_LIBRARY_TYPE

find_path(Halide_ROOT_DIR
          NAMES include/Halide.h
          PATH_SUFFIXES halide)

find_path( Halide_INCLUDE_DIR
           NAMES Halide.h
           HINTS ${Halide_ROOT_DIR}
           PATH_SUFFIXES include
           NO_DEFAULT_PATH
)

if(WIN32)
  set(Halide_LIBRARY_TYPE SHARED)
else()
  set(Halide_LIBRARY_TYPE STATIC)
endif()

if(${Halide_LIBRARY_TYPE} STREQUAL STATIC)
  find_library(Halide_STATIC_LIB Halide HINTS "${Halide_ROOT_DIR}" PATH_SUFFIXES bin)
  mark_as_advanced(Halide_STATIC_LIB)
  set(Halide_LIBRARY ${Halide_STATIC_LIB})
else()
  if(WIN32)
    find_library(Halide_IMPORT_LIB Halide HINTS "${Halide_ROOT_DIR}" PATH_SUFFIXES bin)

    list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .dll)
    find_library(Halide_SHARED_LIB Halide HINTS "${Halide_ROOT_DIR}" PATH_SUFFIXES bin)
    list(REMOVE_AT CMAKE_FIND_LIBRARY_SUFFIXES 0)
  else()
    message(FATAL_ERROR "Halide find module not implemented for shared config on non-win32 platforms")
  endif()

  mark_as_advanced(Halide_SHARED_LIB Halide_IMPORT_LIB)
  set(Halide_LIBRARY ${Halide_SHARED_LIB})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Halide DEFAULT_MSG Halide_ROOT_DIR Halide_INCLUDE_DIR Halide_LIBRARY)

generate_import_target(Halide ${Halide_LIBRARY_TYPE})
