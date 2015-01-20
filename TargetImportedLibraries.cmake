#.rst:
# ImportTargetLibraries
# ---------------------
#
# Created by Walter Gray
# See CreateImportTargetHelpers.cmake for a suite of functions suitable for writing Find
# modules compatible with this approach.
#
# ========================
# TARGET_IMPORTED_LIBRARIES(<target> <link_type> <import_target>)
#   Takes the same arguments as target_link_libraries, but does some additional work
#   to identify and copy any required shared libraries (.dll or .dylib files) to the appropriate
#   location in a custom post-build step.  See documentation at the function definition for more info.
#  TARGET_PACKAGE(<target> <package> ...)
#   Takes the same arguments as find_package, with the addition of the target you're
#   linking to as the first parameter.  Upon successfully finding the package, it
#   attempts to call TARGET_IMPORTED_LIBRARIES(<target> <package>::<package>)
#  VERIFY_LIBRARIES_DEFINED()
#   Verifies that for all targets which were used with target_imported_libraries or target_package
#   All dependencies were resolved and the copy commands were setup properly.

include(CMakeParseArguments)

#helper functions for manipulating target properties.
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


# add_module_copy_command(target dependency):
#   Given a target and a dependency which must be a defined, imported target (type module or shared)
#   Defines a post-build command on the target which will copy the dependency's dll or .dylib file to
#   the same directory as the target's executable.
function(add_module_copy_command target dependency)
  get_target_property(_type ${dependency} TYPE)
  get_target_property(_imported ${dependency} IMPORTED)
  if(NOT ((${_type} STREQUAL SHARED_LIBRARY OR ${_type} STREQUAL MODULE_LIBRARY) AND ${_imported}))
    message(FATAL_ERROR "add_module_copy_command called with target ${target} on bad dependency ${dependency}")
    return()
  endif()

  #if only the _<Config> variants are set, create a generator expression.
  get_target_property(_imported_location ${dependency} IMPORTED_LOCATION)
  if(NOT _imported_location)
    get_target_property(_imported_location_debug ${dependency} IMPORTED_LOCATION_DEBUG)
    get_target_property(_imported_location_release ${dependency} IMPORTED_LOCATION_RELEASE)
    if(NOT _imported_location_debug AND NOT _imported_location_release)
      message(FATAL_ERROR "No IMPORTED_LOCATION specified for SHARED import target ${dependency}")
    endif()
    set(_imported_location "$<$<CONFIG:DEBUG>:${_imported_location_debug}>$<$<CONFIG:RELEASE>:${_imported_location_release}>")
  endif()

  verbose_message("Adding copy command for ${dependency}: ${_imported_location}")

  get_target_property(_install_subdir ${dependency} INSTALL_SUBDIR)
  if(NOT _install_subdir)
    set(_install_subdir)
  else()
    set(_install_subdir /${_install_subdir})
  endif()

  if(MSVC)
    if(_install_subdir)
      add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>${_install_subdir}")
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different \"${_imported_location}\" \"$<TARGET_FILE_DIR:${target}>${_install_subdir}\")
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
endfunction()

function(scan_dependencies_for_dlls target)
  foreach(dependency ${ARGN})
    if(TARGET ${dependency})
      get_target_property(_type ${dependency} TYPE)
      get_target_property(_imported ${dependency} IMPORTED)
      if((${_type} STREQUAL SHARED_LIBRARY OR ${_type} STREQUAL MODULE_LIBRARY) AND ${_imported})
        add_module_copy_command(${target} ${dependency})
      endif()

      get_target_property(_libraries ${dependency} INTERFACE_LINK_LIBRARIES)
      get_target_property(_modules ${dependency} INTERFACE_LINK_MODULES)
      if(NOT _libraries)
        set(_libraries)
      endif()
      if(NOT _modules)
        set(_modules)
      endif()

      #Remove the dependency from the list of unresolved dependencies for this target
      remove_from_prop_set(TARGET ${target} UNRESOLVED_DEPENDENCIES ${dependency})

      scan_dependencies_for_dlls(${target} ${_libraries} ${_modules})
    elseif(NOT dependency MATCHES "^[$-][<-]" AND
           NOT dependency MATCHES ".*[.-]..?.?$")
      #Dependency is not a target and not some freaky generator expression, add to the list of unresolved.
      add_to_prop_set(TARGET ${target} UNRESOLVED_DEPENDENCIES ${dependency})
    endif()
  endforeach()

  #If the target has unresolve dependencies, save it for later
  get_target_property(_target_unresolved ${target} UNRESOLVED_DEPENDENCIES)
  if(_target_unresolved)
    add_to_prop_set(GLOBAL "" UNRESOLVED_TARGETS ${target})
  endif()
endfunction()

function(target_imported_libraries target)
  cmake_parse_arguments(target_imported_libraries "" "LINK_TYPE" "" ${ARGN})

  set(dependencies ${target_imported_libraries_UNPARSED_ARGUMENTS})

  set(_link_lib_list)
  foreach(dependency ${dependencies})
    set(_ismodule FALSE)
    if(TARGET ${dependency})
      get_target_property(_type ${dependency} TYPE)
      if(_type STREQUAL MODULE_LIBRARY)
        set(_ismodule TRUE)
      endif()
    endif()

    if(${_ismodule})
      set_property(TARGET ${target} APPEND PROPERTY INTERFACE_LINK_MODULES ${dependency})
    else()
      list(APPEND _link_lib_list ${dependency})
    endif()

  endforeach()

  target_link_libraries(${target} ${target_imported_libraries_LINK_TYPE} ${_link_lib_list})

  #early out if the target isn't an EXECUTABLE
  get_target_property(_target_type ${target} TYPE)
  if(NOT ${_target_type} STREQUAL EXECUTABLE)
    return()
  endif()

  get_target_property(_libraries ${target} INTERFACE_LINK_LIBRARIES)
  get_target_property(_modules ${target} INTERFACE_LINK_MODULES)
  if(NOT _libraries)
    set(_libraries)
  endif()
  if(NOT _modules)
    set(_modules)
  endif()
  scan_dependencies_for_dlls(${target} ${_libraries} ${_modules})
endfunction()

#This function wraps find_package, then calls target_imported_libraries on the generated package)
function(target_package target package )
  cmake_parse_arguments(target_package "" "LINK_TYPE" "" ${package} ${ARGN})

  if(TARGET ${package}::${package})
    verbose_message("${package}::${package} already exists, skipping find op")
  else()
    find_package(${target_package_UNPARSED_ARGUMENTS})
  endif()

  if(target_package_LINK_TYPE)
    target_imported_libraries(${target} ${package}::${package} LINK_TYPE ${target_package_LINK_TYPE})
  else()
    target_imported_libraries(${target} ${package}::${package})
  endif()
endfunction()

function(scan_unresolved)
  get_property(_global_unresolved GLOBAL PROPERTY UNRESOLVED_TARGETS)
  if(_global_unresolved)
    set(_global_unresolved)
  endif()

  #Do a 2 pass thing so we don't wind up removing unresolved targets added by scan_dependencies_for_dlls
  foreach(target ${_global_unresolved})
    get_target_property(_target_unresolved ${target} UNRESOLVED_DEPENDENCIES)
    verbose_message("found global unresolved target:${target} with dependencies:${_target_unresolved}")
    scan_dependencies_for_dlls(${target} ${_target_unresolved})
    get_target_property(_target_unresolved ${target} UNRESOLVED_DEPENDENCIES)
  endforeach()

  get_property(_global_unresolved GLOBAL PROPERTY UNRESOLVED_TARGETS)
  if(_global_unresolved)
    set(_global_unresolved)
  endif()
  foreach(target ${_global_unresolved})
    get_target_property(_target_unresolved ${target} UNRESOLVED_DEPENDENCIES)
    if(NOT _target_unresolved)
      list(REMOVE_ITEM _global_unresolved ${target})
    endif()
  endforeach()
  set_property(GLOBAL PROPERTY UNRESOLVED_TARGETS ${_global_unresolved})

endfunction()

function(verify_all_copy_steps_resolved)
  scan_unresolved()
  get_property(_global_unresolved GLOBAL PROPERTY UNRESOLVED_TARGETS)
  if(_global_unresolved)
    message(WARNING "Targets with unresolved dependencies remain:\n")
    foreach(target ${_global_unresolved})
      get_target_property(_target_unresolved ${target} UNRESOLVED_DEPENDENCIES)
      message(WARNING "${target}: ${_target_unresolved}")
    endforeach()
  endif()
endfunction()

function(scan_unresolved_EOFHook Variable Access)
  if(${Variable} STREQUAL CMAKE_BACKWARDS_COMPATIBILITY AND
     (${Access} STREQUAL UNKNOWN_READ_ACCESS OR ${Access} STREQUAL READ_ACCESS))
    scan_unresolved()
  endif()
endfunction()

variable_watch(CMAKE_BACKWARDS_COMPATIBILITY scan_unresolved_EOFHook)