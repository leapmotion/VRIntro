include(CreateImportTargetHelpers)

find_path(Protobuf_ROOT_DIR
  NAMES include/google/protobuf/descriptor.h
  HINTS ${EXTERNAL_LIBRARY_DIR}
  PATH_SUFFIXES protobuf-${Protobuf_FIND_VERSION}
                protobuf)

set(Protobuf_INCLUDE_DIR ${Protobuf_ROOT_DIR}/include CACHE STRING "")

find_library(Protobuf_LIBRARY_DEBUG
  NAMES libprotobuf
  HINTS ${Protobuf_ROOT_DIR}/lib/Debug
)

find_library(Protobuf_LIBRARY_RELEASE
  NAMES libprotobuf
  HINTS ${Protobuf_ROOT_DIR}/lib/Release
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Protobuf DEFAULT_MSG
    Protobuf_LIBRARY_RELEASE Protobuf_LIBRARY_DEBUG Protobuf_INCLUDE_DIR)

generate_import_target(Protobuf STATIC)