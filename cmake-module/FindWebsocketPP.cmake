#.rst
# FindWebsocketPP
# ------------
#
# Created by Walter Gray.
# Locate and configure WebsocketPP
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   WebsocketPP::WebsocketPP
#
# Variables
# ^^^^^^^^^
#  WebsocketPP_ROOT_DIR
#  WebsocketPP_FOUND
#  WebsocketPP_INCLUDE_DIR
#  WebsocketPP_LIBRARY_<CONFIG>

set(_suffix "")
if(APPLE)
  set(_suffix "-libc++")
endif()

find_path(WebsocketPP_ROOT_DIR
          NAMES include/websocketpp/websocketpp.hpp
          PATH_SUFFIXES websocketpp${_suffix})

set(WebsocketPP_INCLUDE_DIR ${WebsocketPP_ROOT_DIR}/include)

find_library(WebsocketPP_LIBRARY websocketpp HINTS ${WebsocketPP_ROOT_DIR} PATH_SUFFIXES lib)
set(WebsocketPP_LIBRARY_RELEASE ${WebsocketPP_LIBRARY})

if(WIN32)
  find_library(WebsocketPP_LIBRARY_DEBUG websocketppd HINTS ${WebsocketPP_ROOT_DIR} PATH_SUFFIXES lib)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WebsocketPP DEFAULT_MSG WebsocketPP_INCLUDE_DIR WebsocketPP_LIBRARY)

include(CreateImportTargetHelpers)
generate_import_target(WebsocketPP STATIC)
