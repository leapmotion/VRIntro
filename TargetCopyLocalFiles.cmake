#.rst
# TargetCopyLocalFiles
# --------------------
# Created by Walter Gray.
# Defines functions helpful for copying local files, such as dynamic libraries or resource files
# to the working directory of a project, generally an executable.
#
# =====================
# ADD_LOCAL_FILE_COPY_COMMAND(<target>)
#  Adds the copy command which will reference the REQUIRED_LOCAL_FILES property on the target.
#  The copy command, while platform dependent, will iterate through the list of files, typically
#  DLLS, and will copy them to the appropriate location for the platform.
# ADD_LOCAL_FILES(<target> [FILES <file1> ...] | [DEBUG <file2> ..]  [RELEASE <file3> ...] )
#  Adds files to the list of files to be copied for a target.  TODO: Allow specification of a subdirectory.

#Helper functions for manipulating sets stored in properties.
function(add_to_prop_set scope target property item)
  get_property(_set ${scope} ${target} PROPERTY ${property})
  if(NOT _set)
    set(_set)
  endif()
  list(APPEND _set ${item})
  list(REMOVE_DUPLICATES _set)
  set_property(${scope} ${target} PROPERTY ${property} ${_set})
endfunction()

function(remove_from_prop_set scope target property item)
  get_property(_set ${scope} ${target} PROPERTY ${property})
  if(NOT _set)
    return()
  endif()
  list(REMOVE_ITEM _set ${item})
  set_property(${scope} ${target} PROPERTY ${property} ${_set})
endfunction()

include(CMakeParseArguments)
function(add_local_file_copy_command target)
  get_target_property(_has_command ${target} LOCAL_FILE_COPY_COMMAND_DEFINED)
  if(_has_command)
    return()
  endif()

  if(MSVC)
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND for %i in "(" $<TARGET_PROPERTY:${target},REQUIRED_LOCAL_FILES>;$<$<CONFIG:DEBUG>:$<TARGET_PROPERTY:${target},REQUIRED_LOCAL_FILES_DEBUG>>;$<$<CONFIG:RELEASE>:$<TARGET_PROPERTY:${target},REQUIRED_LOCAL_FILES_RELEASE>> ");do" ${CMAKE_COMMAND} -E copy_if_different \"%i\" \"$<TARGET_FILE_DIR:${target}>\")
  #elseif(APPLE)
  #  get_target_property(_is_bundle ${target} MACOSX_BUNDLE)
  #  if(_is_bundle)
  #    add_custom_command(TARGET ${target} POST_BUILD
  #      COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/../Frameworks/"
  #      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_imported_location}" "$<TARGET_FILE_DIR:${target}>/../Frameworks/"
  #      COMMAND install_name_tool -change @loader_path/libLeap.dylib @loader_path/../Frameworks/libLeap.dylib "$<TARGET_FILE:${target}>")
  #    #call install_name_tool and fixup the dylib paths here:
  #  endif()
  else()
    message(WARNING "Automatic handling of shared libraries is unimplemented on this platform")
  endif()
endfunction()

function(add_local_files target)
  cmake_parse_arguments(add_local_files "" "" "FILES;DEBUG;RELEASE" ${ARGN})

  if(add_local_files_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Invalid arguments passed to add_local_files: ${add_local_files_UNPARSED_ARGUMENTS}")
  endif()

  if(add_local_files_FILES AND (add_local_files_DEBUG OR add_local_files_RELEASE))
    message(FATAL_ERROR "FILES cannot be specified with DEBUG or RELEASE")
  endif()

if(add_local_files_FILES)
  verbose_message("Adding required local files to ${target}: ${add_local_files_FILES}")
  foreach(_file ${add_local_files_FILES})
    add_to_prop_set(TARGET ${target} REQUIRED_LOCAL_FILES ${_file})
  endforeach()
  else()
    if(add_local_files_DEBUG)
      verbose_message("Adding required local debug files to ${target}: ${add_local_files_DEBUG}")
      foreach(_file ${add_local_files_DEBUG})
        add_to_prop_set(TARGET ${target} REQUIRED_LOCAL_FILES_DEBUG ${_file})
      endforeach()
    endif()
    if(add_local_files_RELEASE)
      verbose_message("Adding required local release filesto ${target}: ${add_local_files_RELEASE}")
      foreach(_file ${add_local_files_RELEASE})
        add_to_prop_set(TARGET ${target} REQUIRED_LOCAL_FILES_RELEASE ${_file})
      endforeach()
    endif()
  endif()
endfunction()