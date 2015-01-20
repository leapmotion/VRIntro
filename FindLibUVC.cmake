#.rst
# FindLibUVC
# ------------
#
# Created by Walter Gray.
# Locate and configure LibUVC.
# It provides cmake config files, but at least on mac they are wrong at the time of writing.
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   LibUBC::LibUBC
#
# Variables
# ^^^^^^^^^
#  LibUVC_ROOT_DIR
#  LibUVC_FOUND
#  LibUVC_INCLUDE_DIR
#  LibUVC_LIBRARY


find_path(LibUVC_ROOT_DIR
  NAMES include/libuvc.h
  PATH_SUFFIXES libuvc
)

set(LibUVC_INCLUDE_DIR ${LibUVC_ROOT_DIR}/include)

find_library(LibUVC_LIBRARY 
  NAMES uvc uvc-${LibUVC_FIND_VERSION} HINTS ${LibUVC_ROOT_DIR}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibUVC DEFAULT_MSG LibUVC_INCLUDE_DIR LibUVC_LIBRARY)

include(CreateImportTargetHelpers)
generate_import_target(LibUVC STATIC)
