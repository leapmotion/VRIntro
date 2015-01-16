#.rst
# FindNSS
# ------------
#
# Created by Walter Gray.
# Locate and configure NSS
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   NSS::NSS
#	NSS::certdb
#   NSS::certhi
#   NSS::crmf
#   NSS::cryptohi
#   NSS::dbm
#   NSS::freebl
#   NSS::jar
#   NSS::nspr4
#   NSS::nspr4_s
#   NSS::nss
#   NSS::nss3
#   NSS::nssb
#   NSS::nssckfw
#   NSS::nssdbm
#   NSS::nssdbm3
#   NSS::nssdev
#   NSS::nsspki
#   NSS::nssutil
#   NSS::nssutil3
#   NSS::pk11wrap
#   NSS::pkcs7
#   NSS::pkcs12
#   NSS::pkixcertsel
#   NSS::pkixchecker
#   NSS::pkixcrlsel
#   NSS::pkixmodule
#   NSS::pkixparams
#   NSS::pkixpki
#   NSS::pkixresults
#   NSS::pkixstore
#   NSS::pkixsystem
#   NSS::pkixtop
#   NSS::pkixutil
#   NSS::plc4
#   NSS::plc4_s
#   NSS::plds4
#   NSS::plds4_s
#   NSS::sectool
#   NSS::smime
#   NSS::smime3
#   NSS::softokn
#   NSS::softokn3
#   NSS::sqlite
#   NSS::sqlite3
#   NSS::ssl
#   NSS::ssl3
#   NSS::zlib
#
# Variables
# ^^^^^^^^^
#  NSS_ROOT_DIR
#  NSS_FOUND
#  NSS_<pacakge>_FOUND
#  NSS_INCLUDE_DIR
#  NSS_INTERFACE_LIBS

find_path(NSS_ROOT_DIR
          NAMES include/nss/nss.h
          PATH_SUFFIXES nss-${NSS_FIND_VERSION}
                        nss)

set(NSS_nss_INCLUDE_DIR ${NSS_ROOT_DIR}/include/nss)
set(NSS_nspr4_INCLUDE_DIR ${NSS_ROOT_DIR}/include/nspr)

set(_required_vars NSS_ROOT_DIR NSS_nss_INCLUDE_DIR NSS_nspr4_INCLUDE_DIR)

include(CreateImportTargetHelpers)

#Module find section
set(_modules freebl3 nssckbi)
list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_MODULE_SUFFIX})
foreach(_lib ${_modules})
	find_library(NSS_${_lib}_MODULE ${_lib} HINTS ${NSS_ROOT_DIR} PATH_SUFFIXES bin)
	list(APPEND _required_vars NSS_${_lib}_MODULE)

	if(NOT TARGET NSS::${_lib})
		add_library(NSS::${_lib} MODULE IMPORTED GLOBAL)
		set_property(TARGET NSS::${_lib} PROPERTY IMPORTED_LOCATION ${NSS_${_lib}_MODULE})
	endif()
	list(APPEND NSS_INTERFACE_LIBS NSS::${_lib})
endforeach()
list(REMOVE_AT CMAKE_FIND_LIBRARY_SUFFIXES -1)

#Shared library find section
set(_shared nspr4 nss3 nssdbm3 nssutil3 plc4 plds4 smime3 softokn3 sqlite3 ssl3)
list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})
foreach(_lib ${_shared})
	find_library(NSS_${_lib}_SHARED_LIB ${_lib} HINTS ${NSS_ROOT_DIR} PATH_SUFFIXES bin)
	list(APPEND _required_vars NSS_${_lib}_SHARED_LIB)
	if(WIN32)
		find_library(NSS_${_lib}_IMPORT_LIB ${_lib} HINTS ${NSS_ROOT_DIR} PATH_SUFFIXES lib)
		list(APPEND _required_vars NSS_${_lib}_IMPORT_LIB)
	endif()
	if(NSS_${_lib}_SHARED_LIB)
		set(NSS_${_lib}_FOUND TRUE)
	endif()
	generate_import_target(NSS_${_lib} SHARED TARGET NSS::${_lib})
	list(APPEND NSS_INTERFACE_LIBS NSS::${_lib})
endforeach()
list(REMOVE_AT CMAKE_FIND_LIBRARY_SUFFIXES -1)

#Static library find section
set(_static certdb certhi crmf cryptohi dbm freebl jar nspr4_s nss nssb nssckfw
			nssdbm nssdev nsspki nssutil pk11wrap pkcs7 pkcs12 pkixcertsel pkixchecker
			pkixcrlsel pkixmodule pkixparams pkixpki pkixresults pkixstore pkixsystem
			pkixtop pkixutil plc4_s plds4_s sectool smime softokn sqlite ssl zlib
)
foreach(_lib ${_static})
	find_library(NSS_${_lib}_LIBRARY ${_lib} HINTS ${NSS_ROOT_DIR} PATH_SUFFIXES lib lib/lib)
	list(APPEND _required_vars NSS_${_lib}_LIBRARY)
	if(NSS_${_lib}_LIBRARY)
		set(NSS_${_lib}_FOUND TRUE)
	endif()
	generate_import_target(NSS_${_lib} STATIC TARGET NSS::${_lib})
	list(APPEND NSS_INTERFACE_LIBS NSS::${_lib})
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NSS DEFAULT_MSG ${_required_vars})

generate_import_target(NSS INTERFACE)
