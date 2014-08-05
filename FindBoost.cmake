#.rst:
#FindBoost
#---------
# A FindBoost wrapper that creates import targets

list(APPEND CMAKE_FIND_LIBRARY_PREFIXES "lib")
list(REMOVE_DUPLICATES CMAKE_FIND_LIBRARY_PREFIXES)

if(Boost_FIND_VERSION_EXACT)
  # The version may appear in a directory with or without the patch
  # level, even when the patch level is non-zero.
  set(_boost_TEST_VERSIONS
    "${Boost_FIND_VERSION_MAJOR}.${Boost_FIND_VERSION_MINOR}.${Boost_FIND_VERSION_PATCH}"
    "${Boost_FIND_VERSION_MAJOR}.${Boost_FIND_VERSION_MINOR}")
else()
  # The user has not requested an exact version.  Among known
  # versions, find those that are acceptable to the user request.
  set(_Boost_KNOWN_VERSIONS ${Boost_ADDITIONAL_VERSIONS}
    "1.56.0" "1.56" "1.55.0" "1.55" "1.54.0" "1.54"
    "1.53.0" "1.53" "1.52.0" "1.52" "1.51.0" "1.51"
    "1.50.0" "1.50" "1.49.0" "1.49" "1.48.0" "1.48" "1.47.0" "1.47" "1.46.1"
    "1.46.0" "1.46" "1.45.0" "1.45" "1.44.0" "1.44" "1.43.0" "1.43" "1.42.0" "1.42"
    "1.41.0" "1.41" "1.40.0" "1.40" "1.39.0" "1.39" "1.38.0" "1.38" "1.37.0" "1.37"
    "1.36.1" "1.36.0" "1.36" "1.35.1" "1.35.0" "1.35" "1.34.1" "1.34.0"
    "1.34" "1.33.1" "1.33.0" "1.33")
  set(_boost_TEST_VERSIONS)
  if(Boost_FIND_VERSION)
    set(_Boost_FIND_VERSION_SHORT "${Boost_FIND_VERSION_MAJOR}.${Boost_FIND_VERSION_MINOR}")
    # Select acceptable versions.
    foreach(version ${_Boost_KNOWN_VERSIONS})
      if(NOT "${version}" VERSION_LESS "${Boost_FIND_VERSION}")
        # This version is high enough.
        list(APPEND _boost_TEST_VERSIONS "${version}")
      elseif("${version}.99" VERSION_EQUAL "${_Boost_FIND_VERSION_SHORT}.99")
        # This version is a short-form for the requested version with
        # the patch level dropped.
        list(APPEND _boost_TEST_VERSIONS "${version}")
      endif()
    endforeach()
  else()
    # Any version is acceptable.
    set(_boost_TEST_VERSIONS "${_Boost_KNOWN_VERSIONS}")
  endif()
endif()

foreach(_boost_VER ${_boost_TEST_VERSIONS})
  # Add in a path suffix, based on the required version, ideally
  # we could read this from version.hpp, but for that to work we'd
  # need to know the include dir already
  set(_boost_BOOSTIFIED_VERSION)

  # Transform 1.35 => 1_35 and 1.36.0 => 1_36_0
  if(_boost_VER MATCHES "[0-9]+\\.[0-9]+\\.[0-9]+")
      string(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\1_\\2_\\3"
        _boost_BOOSTIFIED_VERSION ${_boost_VER})
  elseif(_boost_VER MATCHES "[0-9]+\\.[0-9]+")
      string(REGEX REPLACE "([0-9]+)\\.([0-9]+)" "\\1_\\2"
        _boost_BOOSTIFIED_VERSION ${_boost_VER})
  endif()

  list(APPEND _boost_PATH_SUFFIXES
    "boost-${_boost_BOOSTIFIED_VERSION}"
    "boost_${_boost_BOOSTIFIED_VERSION}"
    "boost/boost-${_boost_BOOSTIFIED_VERSION}"
    "boost/boost_${_boost_BOOSTIFIED_VERSION}"
    )

endforeach()

find_path(BOOST_ROOT
    NAMES         include/boost/config.hpp
    PATH_SUFFIXES ${_boost_PATH_SUFFIXES}
    )

include(${CMAKE_ROOT}/Modules/FindBoost.cmake)