#.rst
# FindOculusSDK
# ------------
#
# Locate and configure Oculus
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   FindOculusSDK::FindOculusSDK
#
# Variables
# ^^^^^^^^^
#   OculusSDK_ROOT_DIR
#   OculusSDK_INCLUDE_DIR
#   OculusSDK_LIBRARIES_RELEASE
#   OculusSDK_LIBRARIES_DEBUG
#


find_path(OculusSDK_ROOT_DIR NAMES "LibOVR/Include/OVR.h" PATH_SUFFIXES OculusSDK)

set(OculusSDK_INCLUDE_DIR ${OculusSDK_ROOT_DIR}/LibOVR/Include)

if(MSVC)
  find_library(OculusSDK_LIBRARY_RELEASE "libovr.lib" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/Win32/VS2013" PATH_SUFFIXES lib)
  find_library(OculusSDK_LIBRARY_DEBUG "libovrd.lib" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/Win32/VS2013" PATH_SUFFIXES lib)
elseif(APPLE)
  find_library(OculusSDK_LIBRARY_RELEASE "libovr.a" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/Mac" PATH_SUFFIXES Release)
  find_library(OculusSDK_LIBRARY_DEBUG "libovr.a" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/Mac" PATH_SUFFIXES Debug)
else()
  # Linux's oculus-1.9.0 package's libs are in lib64
  find_library(OculusSDK_LIBRARY_RELEASE "libOCULUS.a" HINTS "${OculusSDK_ROOT_DIR}" PATH_SUFFIXES lib lib64)
  find_library(OculusSDK_LIBRARY_DEBUG "libOCULUS.a" HINTS "${OculusSDK_ROOT_DIR}" PATH_SUFFIXES lib lib64)
endif()
include(SelectConfigurations)
select_configurations(OculusSDK LIBRARY LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OculusSDK DEFAULT_MSG OculusSDK_ROOT_DIR OculusSDK_INCLUDE_DIR OculusSDK_LIBRARY_RELEASE OculusSDK_LIBRARY_DEBUG)

include(CreateImportTargetHelpers)

generate_import_target(OculusSDK STATIC)

if(WIN32)
  #Oculus dev kit relies on winmm and winsock2
  target_link_libraries(OculusSDK::OculusSDK INTERFACE winmm Ws2_32)
endif()