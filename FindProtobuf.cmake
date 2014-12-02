include(CreateImportTargetHelpers)

if(USE_LIBCXX)
 	set(_suffix -libc++)
 endif()

find_path(Protobuf_ROOT_DIR
  NAMES include/google/protobuf/descriptor.h
  HINTS ${EXTERNAL_LIBRARY_DIR}
  PATH_SUFFIXES protobuf-${Protobuf_FIND_VERSION}${_suffix}
                protobuf${_suffix})

set(Protobuf_INCLUDE_DIR ${Protobuf_ROOT_DIR}/include CACHE STRING "")


find_library(Protobuf_LIBRARY
  NAMES libprotobuf
  HINTS ${Protobuf_ROOT_DIR}/lib/Release
  		${Protobuf_ROOT_DIR}/lib
)

find_library(Protobuf_LIBRARY_DEBUG
  NAMES libprotobuf
  HINTS ${Protobuf_ROOT_DIR}/lib/Debug
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Protobuf DEFAULT_MSG
    Protobuf_LIBRARY Protobuf_INCLUDE_DIR)

generate_import_target(Protobuf STATIC)