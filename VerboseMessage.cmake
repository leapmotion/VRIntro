#.rst
#
# VerboseMessage
# --------------
#
# Handy function for printing debug info
function(verbose_message ...)
  if(CMAKE_VERBOSE_MAKEFILE OR VERBOSE)
    message(STATUS ${ARGV})
  endif()
endfunction()