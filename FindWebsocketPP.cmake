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
#  WebsocketPP_LIBRARIES

set(_suffix "")
if(${USE_LIBCXX})
 	set(_suffix "-libc++")
 endif()

find_path(WebsocketPP_ROOT_DIR
          NAMES include/websocketpp/websocketpp.hpp
          PATH_SUFFIXES websocketpp${_suffix})

set(WebsocketPP_INCLUDE_DIR ${WebsocketPP_ROOT_DIR}/include)

find_library(WebsocketPP_LIBRARY_DEBUG websocketppd HINTS ${WebsocketPP_ROOT_DIR} PATH_SUFFIXES lib)
find_library(WebsocketPP_LIBRARY_RELEASE websocketpp HINTS ${WebsocketPP_ROOT_DIR} PATH_SUFFIXES lib)
include(SelectConfigurations)
select_configurations(WebsocketPP LIBRARY LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WebsocketPP DEFAULT_MSG WebsocketPP_INCLUDE_DIR WebsocketPP_LIBRARIES)

include(CreateImportTargetHelpers)
generate_import_target(WebsocketPP STATIC)
