#Our OpenSSL setup is non-standard, so set these variables up beforehand

find_path(OPENSSL_ROOT_DIR
  NAMES include/openssl/opensslconf.h
  PATH_SUFFIXES openssl
)

if(MSVC)
  find_library(OPENSSL_SSL_LIBRARY_DEBUG NAMES libeay32.lib HINTS ${OPENSSL_ROOT_DIR}/lib/debug)
  find_library(OPENSSL_SSL_LIBRARY_RELEASE NAMES libeay32.lib HINTS ${OPENSSL_ROOT_DIR}/lib/release)
  find_library(OPENSSL_CRYPTO_LIBRARY_DEBUG NAMES ssleay32.lib HINTS ${OPENSSL_ROOT_DIR}/lib/debug)
  find_library(OPENSSL_CRYPTO_LIBRARY_RELEASE NAMES ssleay32.lib HINTS ${OPENSSL_ROOT_DIR}/lib/release)
  find_path(OPENSSL_INCLUDE_DIR NAMES openssl/opensslconf.h HINTS ${OPENSSL_ROOT_DIR} PATH_SUFFIXES include)

  mark_as_advanced(OPENSSL_SSL_LIBRARY_DEBUG OPENSSL_SSL_LIBRARY_RELEASE OPENSSL_CRYPTO_LIBRARY_DEBUG OPENSSL_CRYPTO_LIBRARY_RELEASE)

  include(SelectConfigurations)
  select_configurations(OPENSSL_SSL LIBRARY LIBRARIES)
  select_configurations(OPENSSL_CRYPTO LIBRARY LIBRARIES)

else()
  include(${CMAKE_ROOT}/Modules/FindOpenSSL.cmake)
endif()

if(EXISTS "${OPENSSL_CRYPTO_LIBRARY_DEBUG}" OR EXISTS "${OPENSSL_CRYPTO_LIBRARY}")
  set(OPENSSL_CRYPTO_FOUND TRUE)
endif()
if(EXISTS "${OPENSSL_SSL_LIBRARY_DEBUG}" OR EXISTS "${OPENSSL_SSL_LIBRARY}")
  set(OPENSSL_SSL_FOUND TRUE)
endif()
if(OPENSSL_SSL_FOUND AND OPENSSL_CRYPTO_FOUND)
  set(OPENSSL_FOUND TRUE)
endif()

include(CreateImportTargetHelpers)
generate_import_target(OPENSSL_SSL STATIC TARGET OpenSSL::SSL)
generate_import_target(OPENSSL_CRYPTO STATIC TARGET OpenSSL::Crypto)

set(OPENSSL_INTERFACE_LIBS OpenSSL::SSL OpenSSL::Crypto)
generate_import_target(OPENSSL INTERFACE TARGET OpenSSL::OpenSSL)

if(NOT MSVC)
  set_property(TARGET OpenSSL::Crypto APPEND PROPERTY INTERFACE_LINK_LIBRARIES -ldl)
  set_property(TARGET OpenSSL::SSL APPEND PROPERTY INTERFACE_LINK_LIBRARIES -ldl)
endif()
