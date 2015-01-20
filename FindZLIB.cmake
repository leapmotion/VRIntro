#.rst:
# FindZLIB
# --------
#
# Find the native ZLIB includes and library. Created by Walter Gray
#
# Interface Targets
# ^^^^^^^^^^^^^^^^
#  ZLIB::ZLIB`


# Variables
# ^^^^^^^^^^^^^^^^
#
#  ZLIB_INCLUDE_DIRS   
#  ZLIB_LIBRARIES      
#  ZLIB_FOUND          
#
#  ZLIB_VERSION_STRING 
#  ZLIB_VERSION_MAJOR  
#  ZLIB_VERSION_MINOR  
#  ZLIB_VERSION_PATCH  
#  ZLIB_VERSION_TWEAK  


find_path(ZLIB_ROOT_DIR 
  NAMES include/zlib.h 
  PATH_SUFFIXES zlib zlib-${ZLIB_FIND_VERSION} 
)

set(ZLIB_INCLUDE_DIR ${ZLIB_ROOT_DIR}/include)

if(BUILD_64_BIT)
  find_library(ZLIB_LIBRARY NAMES zlib z HINTS ${ZLIB_ROOT_DIR} PATH_SUFFIXES lib lib/x64)
else()
  find_library(ZLIB_LIBRARY NAMES zlib z HINTS ${ZLIB_ROOT_DIR} PATH_SUFFIXES lib lib/x86)
endif()

mark_as_advanced(ZLIB_LIBRARY ZLIB_INCLUDE_DIR)

if(ZLIB_INCLUDE_DIR AND EXISTS "${ZLIB_INCLUDE_DIR}/zlib.h")
    file(STRINGS "${ZLIB_INCLUDE_DIR}/zlib.h" ZLIB_H REGEX "^#define ZLIB_VERSION \"[^\"]*\"$")

    string(REGEX REPLACE "^.*ZLIB_VERSION \"([0-9]+).*$" "\\1" ZLIB_VERSION_MAJOR "${ZLIB_H}")
    string(REGEX REPLACE "^.*ZLIB_VERSION \"[0-9]+\\.([0-9]+).*$" "\\1" ZLIB_VERSION_MINOR  "${ZLIB_H}")
    string(REGEX REPLACE "^.*ZLIB_VERSION \"[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" ZLIB_VERSION_PATCH "${ZLIB_H}")
    set(ZLIB_VERSION_STRING "${ZLIB_VERSION_MAJOR}.${ZLIB_VERSION_MINOR}.${ZLIB_VERSION_PATCH}")

    # only append a TWEAK version if it exists:
    set(ZLIB_VERSION_TWEAK "")
    if( "${ZLIB_H}" MATCHES "ZLIB_VERSION \"[0-9]+\\.[0-9]+\\.[0-9]+\\.([0-9]+)")
        set(ZLIB_VERSION_TWEAK "${CMAKE_MATCH_1}")
        set(ZLIB_VERSION_STRING "${ZLIB_VERSION_STRING}.${ZLIB_VERSION_TWEAK}")
    endif()
endif()

# handle the QUIETLY and REQUIRED arguments and set ZLIB_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(ZLIB REQUIRED_VARS ZLIB_ROOT_DIR ZLIB_LIBRARY ZLIB_INCLUDE_DIR
                                       VERSION_VAR ZLIB_VERSION_STRING)

include(CreateImportTargetHelpers)
generate_import_target(ZLIB STATIC)
