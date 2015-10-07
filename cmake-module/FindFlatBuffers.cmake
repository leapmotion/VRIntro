#.rst
# FindFlatBuffers
# ------------
#
# Created by Walter Gray.
# Locate and configure FlatBuffers
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   FlatBuffers::FlatBuffers
#   FlatBuffers::flatc
#
# Variables
# ^^^^^^^^^
#  FlatBuffers_ROOT_DIR
#  FlatBuffers_FOUND
#  FlatBuffers_INCLUDE_DIR
#  FlatBuffers_FLATC

find_path(FlatBuffers_ROOT_DIR
          NAMES include/flatbuffers/flatbuffers.h
          PATH_SUFFIXES flatbuffers)

set(FlatBuffers_INCLUDE_DIR ${FlatBuffers_ROOT_DIR}/include)

find_program(FlatBuffers_FLATC flatc HINTS ${FlatBuffers_ROOT_DIR} PATH_SUFFIXES bin/${CROSS_COMPILE_EXE_TYPE} bin)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FlatBuffers DEFAULT_MSG FlatBuffers_INCLUDE_DIR FlatBuffers_FLATC)

include(CreateImportTargetHelpers)
generate_import_target(FlatBuffers INTERFACE)
add_executable(FlatBuffers::flatc IMPORTED GLOBAL)
set_property(TARGET FlatBuffers::flatc PROPERTY IMPORTED_LOCATION ${FlatBuffers_FLATC})
