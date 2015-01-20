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

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibUSB DEFAULT_MSG LibUSB_INCLUDE_DIR LibUSB_LIBRARY)

include(CreateImportTargetHelpers)
generate_import_target(LibUSB SHARED)
