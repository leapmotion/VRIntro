#.rst
# FindFreeImage
# ------------
#
# Created by Walter Gray.
# Locate and configure FreeImage
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   FreeImage::FreeImage
#
# Variables
# ^^^^^^^^^
#   FreeImage_ROOT_DIR
#   FreeImage_FOUND
#   FreeImage_INCLUDE_DIR
#   FreeImage_LIBRARIES
#
find_path(FreeImage_ROOT_DIR
          NAMES Dist/FreeImage.h
                include/FreeImage.h
          PATH_SUFFIXES FreeImage-${FreeImage_FIND_VERSION}
                        FreeImage)

find_path(FreeImage_INCLUDE_DIR "FreeImage.h"
          HINTS "${FreeImage_ROOT_DIR}"
          PATH_SUFFIXES Dist include)


if(MSVC)
  find_library(FreeImage_LIBRARY_RELEASE "FreeImage.lib" HINTS "${FreeImage_ROOT_DIR}" PATH_SUFFIXES lib Dist)
  find_library(FreeImage_LIBRARY_DEBUG "FreeImaged.lib" HINTS "${FreeImage_ROOT_DIR}" PATH_SUFFIXES lib Dist)
else()
  find_library(FreeImage_LIBRARY_RELEASE "libfreeimage.dylib" HINTS "${FreeImage_ROOT_DIR}" PATH_SUFFIXES lib Dist)
  find_library(FreeImage_LIBRARY_DEBUG "libfreeimage.dylib" HINTS "${FreeImage_ROOT_DIR}" PATH_SUFFIXES lib Dist)
endif()

include(SelectConfigurations)
select_configurations(FreeImage LIBRARY LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FreeImage DEFAULT_MSG FreeImage_ROOT_DIR FreeImage_INCLUDE_DIR FreeImage_LIBRARY_RELEASE FreeImage_LIBRARY_DEBUG)

include(CreateImportTargetHelpers)

if(MSVC)
  generate_import_target(FreeImage STATIC)
else()
  generate_import_target(FreeImage SHARED)
endif()
