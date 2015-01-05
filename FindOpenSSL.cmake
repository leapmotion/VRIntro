#Our OpenSSL setup is non-standard, so set these variables up beforehand

find_path(OPENSSL_ROOT_DIR
  NAMES include/openssl/opensslconf.h
  PATH_SUFFIXES openssl
)

include(CreateImportTargetHelpers)

if(MSVC)
  find_library(LIB_EAY_DEBUG NAMES libeay32.lib HINTS ${OPENSSL_ROOT_DIR}/lib/debug)
  find_library(LIB_EAY_RELEASE NAMES libeay32.lib HINTS ${OPENSSL_ROOT_DIR}/lib/release)
  find_library(SSL_EAY_DEBUG NAMES ssleay32.lib HINTS ${OPENSSL_ROOT_DIR}/lib/debug)
  find_library(SSL_EAY_RELEASE NAMES ssleay32.lib HINTS ${OPENSSL_ROOT_DIR}/lib/release)
  mark_as_advanced(LIB_EAY_DEBUG LIB_EAY_RELEASE SSL_EAY_DEBUG SSL_EAY_RELEASE)

  include(${CMAKE_ROOT}/Modules/FindOpenSSL.cmake)

  #override the bad OPENSSL_LIBRARIES value
  include(SelectConfigurations)
  select_configurations(LIB_EAY LIBRARY)
  select_configurations(SSL_EAY LIBRARY)
  
  add_library(OpenSSL::SSL STATIC IMPORTED)
  set_target_properties(OpenSSL::SSL PROPERTIES IMPORTED_LOCATION_DEBUG ${LIB_EAY_DEBUG})
  set_target_properties(OpenSSL::SSL PROPERTIES IMPORTED_LOCATION_RELEASE ${LIB_EAY_RELEASE})

  add_library(OpenSSL::Crypto STATIC IMPORTED)
  set_target_properties(OpenSSL::Crypto PROPERTIES IMPORTED_LOCATION_DEBUG ${SSL_EAY_DEBUG})
  set_target_properties(OpenSSL::Crypto PROPERTIES IMPORTED_LOCATION_RELEASE ${SSL_EAY_RELEASE})

  add_library(OpenSSL::OpenSSL INTERFACE IMPORTED GLOBAL)
  target_link_libraries(OpenSSL::OpenSSL INTERFACE OpenSSL::SSL OpenSSL::Crypto)
  set_target_properties(OpenSSL::OpenSSL PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${OPENSSL_INCLUDE_DIR})

else()
  include(${CMAKE_ROOT}/Modules/FindOpenSSL.cmake)
  if(EXISTS "${OPENSSL_CRYPTO_LIBRARY}")
    set(OPENSSL_CRYPTO_FOUND TRUE)
  endif()
  if(EXISTS "${OPENSSL_SSL_LIBRARY}")
    set(OPENSSL_SSL_FOUND TRUE)
  endif()
  
  generate_import_target(OPENSSL_CRYPTO STATIC TARGET OpenSSL::Crypto)
  set_property(TARGET OpenSSL::Crypto APPEND PROPERTY INTERFACE_LINK_LIBRARIES -ldl)
  
  generate_import_target(OPENSSL_SSL STATIC TARGET OpenSSL::SSL)
  set_property(TARGET OpenSSL::SSL APPEND PROPERTY INTERFACE_LINK_LIBRARIES -ldl)

  generate_import_target(OPENSSL INTERFACE TARGET OpenSSL::OpenSSL)
  target_link_libraries(OpenSSL::OpenSSL INTERFACE OpenSSL::SSL OpenSSL::Crypto)
endif()
