#.rst:
# ImportTargetLibraries
# ---------------------
#
# Created by Walter Gray
# See CreateImportTargetHelpers.cmake for a suite of functions suitable for writing Find 
# modules compatible with this approach.
#
# ========================
# TARGET_IMPORTED_LIBRARIES(<target> <import_target>)
#   Takes the same arguments as target_link_libraries, but for any listed library where
#   <import_target> is a valid target with a TYPE property of SHARED_LIBRARY, it will
#   read from the IMPORTED_LOCATION and IMPORTED_LOCATION_<CONFIG> parameters and generate
#   a custom post-build step to copy the shared library files to the appropriate location.
#   On windows, this is the TARGET_FILE_DIR of <target>
#
#  TARGET_PACKAGE(<target> <package> ...)
#   Takes the same arguments as find_package, with the addition of the target you're
#   linking to as the first parameter.  Upon successfully finding the package, it
#   attempts to call TARGET_IMPORTED_LIBRARIES(<target> <package>::<package>)

function(target_imported_libraries target)
  list(REMOVE_AT ARGV 0) #pop the target
  
  foreach(_import_lib ${ARGV})
    if(TARGET ${_import_lib})
      target_link_libraries(${target} ${_import_lib})
      
      get_target_property(_type ${_import_lib} TYPE)
      get_target_property(_imported ${_import_lib} IMPORTED)
      if((${_type} STREQUAL SHARED_LIBRARY) AND ${_imported})
        
        set(_found_configs_expr)
        set(_target_expr)
        foreach(_config DEBUG RELEASE)
          set(_config_suffix _${_config})
          
          get_target_property(_dylib${_config} ${_import_lib} IMPORTED_LOCATION${_config_suffix})
          if(_dylib${_config})
            list(APPEND _found_configs_expr "$<CONFIG:${_config}>")
            set(_target_expr "${_target_expr}$<$<CONFIG:${_config}>:${_dylib${_config}}>")
          endif()
        endforeach()
        
        #The default case requires special handling
        get_target_property(_dylib ${_import_lib} IMPORTED_LOCATION)
        if(_dylib)
          if(_found_configs_expr)
              #For some reason, generator expressions require their lists to be , delimited
              string(REPLACE ";" "," _found_configs_expr "${_found_configs_expr}")
            set(_target_expr "${_target_expr}$<$<NOT:$<OR:${_found_configs_expr}>>:${_dylib}>")
          else()
            set(_target_expr "${_dylib}")
          endif()
        endif()
          
        if(NOT _target_expr)
          message(FATAL_ERROR "No dylib specified for SHARED import target ${_import_lib}")
        endif()
        
        verbose_message("Adding copy command for ${_import_lib}: ${_target_expr}")

        if(MSVC)
          add_custom_command(TARGET ${target} POST_BUILD 
            COMMAND ${CMAKE_COMMAND} -E copy_if_different \"${_target_expr}\" \"$<TARGET_FILE_DIR:${target}>\")
        elseif(APPLE)
          add_custom_command(TARGET ${target} POST_BUILD 
            COMMAND ${CMAKE_COMMAND} -E copy_if_different \"${_target_expr}\" \"$<TARGET_FILE_DIR:${target}>/../Frameworks\")
          #call install_name_tool and fixup the dylib paths here:
        else()
          message(WARNING "Automatic handling of shared libraries is unimplemented on this platform")
        endif()
      endif()
    endif()
  endforeach()
endfunction()

#This function wraps find_package, then calls target_imported_libraries on the generated package)
function(target_package target package)
  list(REMOVE_AT ARGV 0) # pop the target
  find_package(${ARGV})
  target_imported_libraries(${target} ${package}::${package})
endfunction()
