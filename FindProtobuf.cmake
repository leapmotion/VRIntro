include(CreateImportTargetHelpers)

message(USE_LIBCXX=${USE_LIBCXX})
set(_suffix "")

if(${USE_LIBCXX})
	message("Setting protobuf suffix")
 	set(_suffix "-libc++")
 endif()

unset(Protobuf_ROOT_DIR CACHE)
unset(Protobuf_LIBRARY CACHE)
unset(Protobuf_LIBRARY_DEBUG CACHE)

message(_suffix=${_suffix})
find_path(Protobuf_ROOT_DIR
  NAMES include/google/protobuf/descriptor.h
  HINTS ${EXTERNAL_LIBRARY_DIR}
  PATH_SUFFIXES protobuf-${Protobuf_FIND_VERSION}${_suffix}
                protobuf${_suffix})

set(Protobuf_INCLUDE_DIR ${Protobuf_ROOT_DIR}/include CACHE STRING "")
message(CMAKE_FIND_LIBRARY_SUFFIXES=${CMAKE_FIND_LIBRARY_SUFFIXES})


find_library(Protobuf_LIBRARY
  NAMES libprotobuf
  HINTS "${Protobuf_ROOT_DIR}/lib"
  		"${Protobuf_ROOT_DIR}/lib/Release"
)
message(Protobuf_LIBRARY=${Protobuf_LIBRARY})
message(searchedin="${Protobuf_ROOT_DIR}/lib")

find_library(Protobuf_LIBRARY_DEBUG
  NAMES libprotobuf
  HINTS ${Protobuf_ROOT_DIR}/lib/Debug
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Protobuf DEFAULT_MSG
    Protobuf_LIBRARY Protobuf_INCLUDE_DIR)

generate_import_target(Protobuf STATIC)