#.rst:
#FindBoost
#---------
# A FindBoost wrapper that creates import targets

if(WIN32)
  list(APPEND CMAKE_FIND_LIBRARY_PREFIXES "lib")
  list(REMOVE_DUPLICATES CMAKE_FIND_LIBRARY_PREFIXES)
endif()

include(${CMAKE_ROOT}/Modules/FindBoost.cmake)