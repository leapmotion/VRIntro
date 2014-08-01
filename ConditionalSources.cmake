#.rst
#ConditionalSources
#---------------
#
# This module defines a set of functions for specifying platform specific source files.
# When using these, all source files regardless of platform should be passed to the add_* commands
# These commands can be used either before or after the add_* command.

# Specifies a set of source files that are only meant to be incldued given a certain condition,
# such as WIN32, or MSVC.
function(conditional_sources condition_var ...)
  include(CMakeParseArguments)
  cmake_parse_arguments(conditional_sources "" "GROUP_NAME" "FILES" ${ARGV})

  source_group(${conditional_sources_GROUP_NAME} FILES ${conditional_sources_FILES})

  message(STATUS "Setting source group ${conditional_sources_GROUP_NAME} with files ${conditional_sources_FILES}")
  if(NOT ${condition_var})
    set_source_files_properties( ${ARGN} PROPERTIES HEADER_FILE_ONLY TRUE)
  endif()
endfunction()

# A small set of convienence functions for common conditions/group names
function(windows_sources ...)
  conditional_sources(WIN32 GROUP_NAME "Windows Source" FILES ${ARGV})
endfunction()

function(mac_sources ...)
  conditional_sources(WIN32 GROUP_NAME "Mac Source" FILES ${ARGV})
endfunction()

function(unix_sources ...)
  conditional_sources(WIN32 GROUP_NAME "Unix Source" FILES ${ARGV})
endfunction()


#as conditional_sources, but also appends the soruces to the source_list_var
function(add_conditional_sources source_list_var condition_var ...)
  list(REMOVE_AT ARGV 0)
  conditional_sources(${ARGV})

  include(CMakeParseArguments)
  cmake_parse_arguments(add_conditional_sources "" "" "FILES" ${ARGV})
  set(${source_list_var} ${${source_list_var}} ${add_conditional_sources_FILES} PARENT_SCOPE)
endfunction()

function(add_windows_sources source_list_var ...)
  list(REMOVE_AT ARGV 0)
  add_conditional_sources(${source_list_var} WIN32 GROUP_NAME "Windows Source" FILES ${ARGV})
  set(${source_list_var} ${${source_list_var}} PARENT_SCOPE)
endfunction()

function(add_mac_sources source_list_var ...)
  list(REMOVE_AT ARGV 0)
  add_conditional_sources(${source_list_var} WIN32 GROUP_NAME "Mac Source" FILES ${ARGV})
  set(${source_list_var} ${${source_list_var}} PARENT_SCOPE)
endfunction()

function(unix_sources source_list_var ...)
  list(REMOVE_AT ARGV 0)
  conditional_sources(${source_list_var} WIN32 GROUP_NAME "Unix Source" FILES ${ARGV})
  set(${source_list_var} ${${source_list_var}} PARENT_SCOPE)
endfunction()
