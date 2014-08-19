#.rst
# FindPoly2Tri
# ------------
#
# Created by Jonathan Marsden
# Locate and configure Poly2Tri
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   Poly2Tri::Poly2Tri
#
# Variables
# ^^^^^^^^^
#   Poly2Tri_ROOT_DIR
#   Poly2Tri_FOUND
#   Poly2Tri_INCLUDE_DIR
#   Poly2Tri_LIBRARY
#   Poly2Tri_IMPORT_LIB

find_path(Poly2Tri_ROOT_DIR
          NAMES include/poly2tri.h
          HINTS ${EXTERNAL_LIBRARY_DIR}
          PATH_SUFFIXES poly2tri)

set(Poly2Tri_INCLUDE_DIR "${Poly2Tri_ROOT_DIR}/include")
if(MSVC)
  find_library(Poly2Tri_LIBRARY_RELEASE "poly2tri.lib" HINTS "${Poly2Tri_ROOT_DIR}/lib/release")
  find_library(Poly2Tri_LIBRARY_DEBUG "poly2tri.lib" HINTS "${Poly2Tri_ROOT_DIR}/lib/debug")
else()
  find_library(Poly2Tri_LIBRARY_RELEASE "libpoly2tri.a" HINTS "${Poly2Tri_ROOT_DIR}/lib")
  find_library(Poly2Tri_LIBRARY_DEBUG "libpoly2tri.a" HINTS "${Poly2Tri_ROOT_DIR}/lib")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Poly2Tri DEFAULT_MSG Poly2Tri_ROOT_DIR Poly2Tri_INCLUDE_DIR Poly2Tri_LIBRARY_RELEASE Poly2Tri_LIBRARY_DEBUG)

include(CreateImportTargetHelpers)
generate_import_target(Poly2Tri STATIC)
