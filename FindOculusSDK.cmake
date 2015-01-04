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

#We include the src directory as well since we need OVR_CAPI_GL/D3D
set(OculusSDK_INCLUDE_DIR "${OculusSDK_ROOT_DIR}/LibOVR/Include" "${OculusSDK_ROOT_DIR}/LibOVR/Src")

if(MSVC)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(Oculus_ARCHITECTURE x64)
    set(Oculus_ARCHITECTURE_SUFFIX 64)
  else()
    set(Oculus_ARCHITECTURE Win32)
    set(Oculus_ARCHITECTURE_SUFFIX)
  endif()

  find_library(OculusSDK_LIBRARY_RELEASE "libovr${Oculus_ARCHITECTURE_SUFFIX}.lib" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/${Oculus_ARCHITECTURE}/VS2013" PATH_SUFFIXES lib)
  find_library(OculusSDK_LIBRARY_DEBUG "libovr${Oculus_ARCHITECTURE_SUFFIX}d.lib" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/${Oculus_ARCHITECTURE}/VS2013" PATH_SUFFIXES lib)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # Mac
  find_library(OculusSDK_LIBRARY_RELEASE "libovr.a" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/Mac/Release" PATH_SUFFIXES lib)
  find_library(OculusSDK_LIBRARY_DEBUG "libovr.a" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/Mac/Debug" PATH_SUFFIXES lib)
else()
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(Oculus_ARCHITECTURE x86_64)
  else()
    set(Oculus_ARCHITECTURE x86)
  endif()
  find_library(OculusSDK_LIBRARY_RELEASE "libovr.a" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/Linux/Release/${Oculus_ARCHITECTURE}" PATH_SUFFIXES lib)
  find_library(OculusSDK_LIBRARY_DEBUG "libovr.a" HINTS "${OculusSDK_ROOT_DIR}/LibOVR/Lib/Linux/Debug/${Oculus_ARCHITECTURE}" PATH_SUFFIXES lib)
  if(OculusSDK_LIBRARY_RELEASE AND NOT OculusSDK_LIBRARY_DEBUG)
    set(OculusSDK_LIBRARY_DEBUG ${OculusSDK_LIBRARY_RELEASE})
  endif()
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
