#.rst
#ConditionalSources
#---------------
#
# This module defines a set of functions for specifying platform specific source files.
# When using these, all source files regardless of platform should be passed to the add_* commands
# These commands can be used either before or after the add_* command.

# Specifies a set of source files that are only meant to be incldued given a certain condition,
# such as WIN32, or MSVC.
include(VerboseMessage)

function(conditional_sources condition_var ...)
  include(CMakeParseArguments)
  cmake_parse_arguments(conditional_sources "" "GROUP_NAME" "FILES" ${ARGV})

  source_group(${conditional_sources_GROUP_NAME} FILES ${conditional_sources_FILES})

  if(NOT (${condition_var}))
    set_source_files_properties( ${ARGN} PROPERTIES HEADER_FILE_ONLY TRUE)
    verbose_message("Setting INACTIVE source group \"${conditional_sources_GROUP_NAME}\" with files ${conditional_sources_FILES}")
  else()
    verbose_message("Setting source group \"${conditional_sources_GROUP_NAME}\" with files ${conditional_sources_FILES}")
  endif()
endfunction()

#as conditional_sources, but also appends the soruces to the source_list_var
function(add_conditional_sources source_list_var condition_var ...)
  list(REMOVE_AT ARGV 0)
  conditional_sources(${ARGV})

  include(CMakeParseArguments)
  cmake_parse_arguments(add_conditional_sources "" "" "FILES" ${ARGV})
  set(${source_list_var} ${${source_list_var}} ${add_conditional_sources_FILES} PARENT_SCOPE)
endfunction()

macro(_add_platform_conditionals platform condition)
  function(${platform}_sources ...)
    set(my_argv ARGV)
    conditional_sources("${condition}" GROUP_NAME "${platform} Source" FILES ${${my_argv}})
  endfunction()

  function(add_${platform}_sources source_list_var ...)
    set(my_argv ARGV)

    list(REMOVE_AT ARGV 0)
    add_conditional_sources(${source_list_var} "${condition}" GROUP_NAME "${platform} Source" FILES ${${my_argv}})
    set(${source_list_var} ${${source_list_var}} PARENT_SCOPE)
  endfunction()
endmacro()

_add_platform_conditionals(Windows WIN32)
_add_platform_conditionals(Mac APPLE)
_add_platform_conditionals(Unix "UNIX AND NOT APPLE AND NOT WIN32")
