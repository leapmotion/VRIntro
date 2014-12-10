#.rst:
# CopySharedLibraries
# ---------------------
#
# Created by Walter Gray
# See CreateImportTargetHelpers.cmake for a suite of functions suitable for writing Find
# modules compatible with this approach.
#
# ========================
# copy_shared_libraries(<target> <explicit dependenencies>)
#   Scans a target exe's INTERFACE_LINK_LIBRARIES searching for targets in the heirarchy
#   with TYPE set to SHARED_LIBRARY. When found, it will read from the IMPORTED_LOCATION
#   and IMPORTED_LOCATION_<CONFIG> parameters to generate a custom post-build step to copy
#   the shared library files to the appropriate location.
#   On Windows, this is the TARGET_FILE_DIR of <target>. On Mac, it will detect if the target
#   is a bundle and will place the .dylibs in the correct location.
#   Note that all dependencies must be defined at the time of this function call

include(CMakeParseArguments)

function(expand_interface_libraries outvar target)
  get_target_property(targetprops ${target} INTERFACE_LINK_LIBRARIES)
  foreach(lib ${targetprops})
    list(FIND ${outvar} ${lib} _found)
    if(${_found} EQUAL -1)
      list(APPEND ${outvar} ${lib})
      if(TARGET ${lib})
        expand_interface_libraries(${outvar} ${lib})
      endif()
    endif()
  endforeach()
  set(${outvar} ${${outvar}} PARENT_SCOPE)
endfunction()

function(copy_shared_libraries target)
  #early out if the target isn't an EXECUTABLE
  get_target_property(_target_type ${target} TYPE)
  if(NOT ${_target_type} STREQUAL EXECUTABLE)
    return()
  endif()

  expand_interface_libraries(libraries ${target})

  list(APPEND libraries ${ARGN})

  foreach(lib ${libraries})
    if(TARGET ${lib})
      get_target_property(_type ${lib} TYPE)
      get_target_property(_imported ${lib} IMPORTED)
      if(${_type} STREQUAL SHARED_LIBRARY AND ${_imported})
        #if only the _<Config> variants are set, create a generator expression.
        set(_imported_location)
        get_target_property(_imported_location ${lib} IMPORTED_LOCATION)
        if(NOT _imported_location)
          get_target_property(_imported_location_debug ${lib} IMPORTED_LOCATION_DEBUG)
          get_target_property(_imported_location_release ${lib} IMPORTED_LOCATION_RELEASE)
          if(NOT _imported_location_debug AND NOT _imported_location_release)
            message(FATAL_ERROR "No IMPORTED_LOCATION specified for SHARED import target ${lib}")
          endif()
          set(_imported_location "$<$<CONFIG:DEBUG>:${_imported_location_debug}>$<$<CONFIG:RELEASE>:${_imported_location_release}>")
        endif()

        verbose_message("Adding copy command for ${lib}: ${_imported_location}")

        if(WIN32)
          add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different \"${_imported_location}\" \"$<TARGET_FILE_DIR:${target}>\")
        elseif(APPLE)
          get_target_property(_is_bundle ${target} MACOSX_BUNDLE)
          if(_is_bundle)
            add_custom_command(TARGET ${target} POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/../Frameworks/"
              COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_imported_location}" "$<TARGET_FILE_DIR:${target}>/../Frameworks/"
              COMMAND install_name_tool -change @loader_path/libLeap.dylib @loader_path/../Frameworks/libLeap.dylib "$<TARGET_FILE:${target}>")
            #call install_name_tool and fixup the dylib paths here:
          endif()
        else()
          message(WARNING "Automatic handling of shared libraries is unimplemented on this platform")
        endif()

      endif()
    endif()
  endforeach()
endfunction()
