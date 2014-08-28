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
#   Oculus_ROOT_DIR
#   Oculus_FOUND
#   Oculus_INCLUDE_DIR
#   Oculus_LIBRARIES
#


find_path(OculusSDK_ROOT_DIR NAMES "LibOVR/Include/OVR.h" PATH_SUFFIXES OculusSDK)

set(OculusSDK_INCLUDE_DIR ${OculusSDK_ROOT_DIR}/LibOVR/Include)
# set(OculusSDK_INCLUDE_DIR "${OculusSDK_INCLUDE_DIR} ${OculusSDK_ROOT_DIR}/LibOVR/Src")
include_directories(
	${OculusSDK_ROOT_DIR}/LibOVR/Src
	${OculusSDK_ROOT_DIR}/3rdParty/glext  # this is a temp hack before we start using component
	)

if(MSVC)
  find_library(OculusSDK_LIBRARY_RELEASE "libovr.lib" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/Win32/VS2013" PATH_SUFFIXES lib)
  find_library(OculusSDK_LIBRARY_DEBUG "libovrd.lib" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/Win32/VS2013" PATH_SUFFIXES lib)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # Mac
  find_library(OculusSDK_LIBRARY_RELEASE "libovr.a" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/Mac/Release" PATH_SUFFIXES lib)
  find_library(OculusSDK_LIBRARY_DEBUG "libovr.a" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/Mac/Debug" PATH_SUFFIXES lib)
else()
  # No linux support from Oculus Rift!
endif()
include(SelectConfigurations)
select_configurations(OculusSDK LIBRARY LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OculusSDK DEFAULT_MSG OculusSDK_ROOT_DIR OculusSDK_INCLUDE_DIR OculusSDK_LIBRARY_RELEASE OculusSDK_LIBRARY_DEBUG)

include(CreateImportTargetHelpers)

generate_import_target(OculusSDK STATIC)
