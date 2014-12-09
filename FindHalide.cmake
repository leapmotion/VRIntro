#.rst
# FindHalide
# ------------
#
# Created by Walter Gray
# Locate and configure Halide
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
#   Halide_SHARED_LIB
#   Halide_IMPORT_LIB

find_path(Halide_ROOT_DIR
          NAMES include/Halide.h
          PATH_SUFFIXES halide)

find_path( Halide_INCLUDE_DIR
           NAMES Halide.h
           HINTS ${Halide_ROOT_DIR}
           PATH_SUFFIXES include
           NO_DEFAULT_PATH
)

find_library(Halide_LIBRARY Halide HINTS "${Halide_ROOT_DIR}" PATH_SUFFIXES bin)
if(WIN32)
  set(Halide_IMPORT_LIB ${Halide_LIBRARY})
	find_library(Halide_SHARED_LIB Halide.dll "$[Halide_ROOT_DIR}" PATH_SUFFIXES bin)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Halide DEFAULT_MSG Halide_INCLUDE_DIR Halide_LIBRARY)

include(CreateImportTargetHelpers)
if(WIN32)
	generate_import_target(Halide SHARED)
else()
	generate_import_target(Halide STATIC)
endif()