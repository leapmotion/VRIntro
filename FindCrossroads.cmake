#.rst
# FindCrossroads
# ------------
#
# Created by Walter Gray.
# Locate and configure Crossroads
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   Crossroads::Crossroads
#
# Variables
# ^^^^^^^^^
#  Crossroads_ROOT_DIR
#  Crossroads_FOUND
#  Crossroads_INCLUDE_DIR
#  Crossroads_LIBRARIES
#  Crossroads_LIBRARY_RELEASE
#  Crossroads_LIBRARY_DEBUG


find_path(Crossroads_ROOT_DIR
	NAMES include/xs/xs.h
	PATH_SUFFIXES libxs-${Crossroads_FIND_VERSION}-${ALTERNATE_LIBRARY}
				  libxs-${Crossroads_FIND_VERSION}
				  libxs
	)

set(Crossroads_INCLUDE_DIR ${Crossroads_ROOT_DIR}/include)

if(WIN32)
  if(BUILD_64_BIT)
    find_library(Crossroads_LIBRARY_DEBUG "libxs_d.lib" HINTS ${Crossroads_ROOT_DIR}/lib/x64)
    find_library(Crossroads_LIBRARY_RELEASE "libxs.lib" HINTS ${Crossroads_ROOT_DIR}/lib/x64)
  else()
  	find_library(Crossroads_LIBRARY_DEBUG "libxs_d.lib" HINTS ${Crossroads_ROOT_DIR}/lib/Win32)
    find_library(Crossroads_LIBRARY_RELEASE "libxs.lib" HINTS ${Crossroads_ROOT_DIR}/lib/Win32)
  endif()

  include(SelectConfigurations)
  select_configurations(Crossroads LIBRARY LIBRARIES)
else()
  find_library(Crossroads_LIBRARY xs HINTS ${Crossroads_ROOT_DIR}/lib)
  set(Crossroads_LIBRARIES ${Crossroads_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Crossroads DEFAULT_MSG Crossroads_INCLUDE_DIR Crossroads_LIBRARIES)

include(CreateImportTargetHelpers)
generate_import_target(Crossroads STATIC)
