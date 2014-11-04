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

#defines 'func_name' and add_'func_name' shorthands for add_conditional_sources with pre-set conditions.
macro(add_named_conditional_functions func_name group_name condition)
  function(${func_name} ...)
    set(my_argv ARGV)
    conditional_sources("${condition}" GROUP_NAME "${group_name}" FILES ${${my_argv}})
  endfunction()

  function(add_${func_name} source_list_var ...)
    set(my_argv ARGV)

    list(REMOVE_AT ARGV 0)
    add_conditional_sources(${source_list_var} "${condition}" GROUP_NAME "${group_name}" FILES ${${my_argv}})
    set(${source_list_var} ${${source_list_var}} PARENT_SCOPE)
  endfunction()
endmacro()

#Some good defaults
add_named_conditional_functions("windows_sources" "Windows Source" WIN32)
add_named_conditional_functions("mac_sources" "Mac Source" APPLE)
add_named_conditional_functions("unix_sources" "Unix Source" "UNIX AND NOT APPLE AND NOT WIN32")
add_named_conditional_functions("resource_files" "Resource Files" FALSE)
