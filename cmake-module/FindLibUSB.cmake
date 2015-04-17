#.rst
# FindLibUSB
# ------------
#
# Created by Walter Gray.
# Locate and configure LibUSB
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   LibUSB::LibUSB
#
# Variables
# ^^^^^^^^^
#  LibUSB_ROOT_DIR
#  LibUSB_FOUND
#  LibUSB_INCLUDE_DIR
#  LibUSB_LIBRARY


find_path(LibUSB_ROOT_DIR
  NAMES include/libusb/libusb.h
  PATH_SUFFIXES libusb
)

set(LibUSB_INCLUDE_DIR ${LibUSB_ROOT_DIR}/include)

find_library(LibUSB_LIBRARY
  NAMES usb usb-${LibUSB_FIND_VERSION} HINTS ${LibUSB_ROOT_DIR}/lib)

#This is a little bit of a hack - if this becomes a common use-case we may need
#to add the ability to specify destination file names to add_local_files
if(BUILD_LINUX AND NOT BUILD_ANDROID AND NOT LibUSB_LIBRARY_ORIGINAL)
  set(LibUSB_LIBRARY_ORIGINAL ${LibUSB_LIBRARY} CACHE FILEPATH "")
  mark_as_advanced(LibUSB_LIBRARY_ORIGINAL)

  get_filename_component(_basename "${LibUSB_LIBRARY}" NAME_WE)
  set(LibUSB_LIBRARY ${CMAKE_BINARY_DIR}/libusb-temp/${_basename}.0${CMAKE_SHARED_LIBRARY_SUFFIX}.0 CACHE FILEPATH "" FORCE)

  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/libusb-temp)
  configure_file(${LibUSB_LIBRARY_ORIGINAL} ${LibUSB_LIBRARY} COPYONLY)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibUSB DEFAULT_MSG LibUSB_INCLUDE_DIR LibUSB_LIBRARY)

include(CreateImportTargetHelpers)
generate_import_target(LibUSB SHARED)
