include(CreateImportTargetHelpers)

set(_suffix "")
if(${USE_LIBCXX})
 	set(_suffix "-libc++")
 endif()

find_path(Protobuf_ROOT_DIR
  NAMES include/google/protobuf/descriptor.h
  HINTS ${EXTERNAL_LIBRARY_DIR}
  PATH_SUFFIXES protobuf-${Protobuf_FIND_VERSION}${_suffix}
                protobuf${_suffix})

set(Protobuf_INCLUDE_DIR ${Protobuf_ROOT_DIR}/include CACHE STRING "")

find_library(Protobuf_LIBRARY
  NAMES protobuf libprotobuf #This is dumb, but nessecary because on mac, the prefix is "lib", but on windows its ""
  HINTS "${Protobuf_ROOT_DIR}/lib"
  		"${Protobuf_ROOT_DIR}/lib/Release"
)

find_library(Protobuf_LIBRARY_DEBUG
  NAMES protobuf libprotobuf
  HINTS ${Protobuf_ROOT_DIR}/lib/Debug
)
mark_as_advanced(Protobuf_LIBRARY_DEBUG)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Protobuf DEFAULT_MSG
    Protobuf_LIBRARY Protobuf_INCLUDE_DIR)

generate_import_target(Protobuf STATIC)