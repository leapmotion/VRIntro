#.rst
# FindFreespace
# ------------
#
# Locate and configure freespace
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   FindFreespace::FindFreespace
#
# Variables
# ^^^^^^^^^
#   Freespace_FOUND
#   Freespace_ROOT_DIR
#   Freespace_INCLUDE_DIR
#   Freespace_LIBRARIES_RELEASE
#   Freespace_LIBRARIES_DEBUG
#   Freespace_DEBUG

set(Freespace_FOUND)
macro(Freespace_MSG msg)
  if(Freespace_DEBUG)
    message(STATUS ${msg})
  endif()
endmacro()

find_path(Freespace_ROOT_DIR NAMES "include/freespace/freespace.h" PATH_SUFFIXES "libfreespace-${Freespace_FIND_VERSION}")
if(Freespace_ROOT_DIR)
  Freespace_MSG("Freespace root found at ${Freespace_ROOT_DIR}")
else()
  Freespace_MSG("Could not find freespace root directory ${Freespace_ROOT_DIR}")
endif()

# Include directory is in the standard format
set(Freespace_INCLUDE_DIR "${Freespace_ROOT_DIR}/include")

# Library has a standad name, too, for both debug and release
find_library(Freespace_LIBRARY_RELEASE "libfreespace" HINTS "${Freespace_ROOT_DIR}" PATH_SUFFIXES lib)
find_library(Freespace_LIBRARY_DEBUG "libfreespaced.lib" HINTS "${Freespace_ROOT_DIR}" PATH_SUFFIXES lib)
Freespace_MSG("Freespace release library ${Freespace_LIBRARY_RELEASE}")
Freespace_MSG("Freespace debug library ${Freespace_LIBRARY_DEBUG}")

include(SelectConfigurations)
select_configurations(Freespace LIBRARY LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Freespace DEFAULT_MSG Freespace_ROOT_DIR Freespace_INCLUDE_DIR Freespace_LIBRARY_RELEASE Freespace_LIBRARY_DEBUG)

include(CreateImportTargetHelpers)

generate_import_target(Freespace STATIC)
set(Freespace_FOUND ON)
