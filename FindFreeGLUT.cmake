#.rst
# FindFreeGLUT
# ------------
#
# Created by Walter Gray.
# Locate and configure FreeGLUT
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   FreeGLUT::FreeGLUT
#
# Variables
# ^^^^^^^^^
#  FreeGLUT_ROOT_DIR
#  FreeGLUT_FOUND
#  FreeGLUT_INCLUDE_DIR
#  FreeGLUT_LIBRARY


find_path(FreeGLUT_ROOT_DIR
	NAMES include/GL/freeglut.h
	PATH_SUFFIXES freeglut-${FreeGLUT_FIND_VERSION}
				  freeglut
	)

set(FreeGLUT_INCLUDE_DIR ${FreeGLUT_ROOT_DIR}/include)

if(BUILD_WINDOWS)
  if(BUILD_64_BIT)
    find_library(FreeGLUT_LIBRARY "freeglut_static.lib" HINTS ${FreeGLUT_ROOT_DIR}/lib/x64)
  else()
    find_library(FreeGLUT_LIBRARY "freeglut_static.lib" HINTS ${FreeGLUT_ROOT_DIR}/lib/x86)
  endif()
elseif(NOT BUILD_MAC) # Linux
  if(BUILD_ANDROID)
    find_library(FreeGLUT_LIBRARY freeglut-gles2 HINTS ${FreeGLUT_ROOT_DIR}/lib)
  else()
    find_library(FreeGLUT_LIBRARY glut HINTS ${FreeGLUT_ROOT_DIR}/lib)
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FreeGLUT DEFAULT_MSG FreeGLUT_INCLUDE_DIR FreeGLUT_LIBRARY)

include(CreateImportTargetHelpers)
generate_import_target(FreeGLUT STATIC)
