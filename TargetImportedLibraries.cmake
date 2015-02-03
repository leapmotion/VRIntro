#.rst:
# ImportTargetLibraries
# ---------------------
#
# Created by Walter Gray
# See CreateImportTargetHelpers.cmake for a suite of functions suitable for writing Find
# modules compatible with this approach.
# Makes use of the TargetCopyLocalFiles module to copy any dependant shared libraries at build time.
#
# ========================
#  TARGET_COPY_SHARED_LIBRARIES(<target>)
#   Given a target, will cause all dependent shared libraries to be copied to the appropriate
#   location: the local directory on windows or the resources folder for a mac .app package.
#  TARGET_IMPORTED_LIBRARIES(<target> <link_type> <import_target>) - LEGACY
#   Takes the same arguments as target_link_libraries, but does some additional work
#   to identify and copy any required shared libraries (.dll or .dylib files) to the appropriate
#   location in a custom post-build step. See documentation at the function definition for more info.
#  TARGET_PACKAGE(<target> <package> ...) - LEGACY
#   Takes the same arguments as find_package, with the addition of the target you're
#   linking to as the first parameter. Upon successfully finding the package, it
#   attempts to call TARGET_IMPORTED_LIBRARIES(<target> <package>::<package>)
#  VERIFY_SHARED_LIBRARIES_RESOLVED()
#   Verifies that for all targets which were used with target_imported_libraries or target_package
#   All dependencies were resolved and the copy commands were setup properly.
#   Not strictly nessecary thanks to the variable_watch on CMAKE_BACKWARDS_COMPATIBILITY, but
#   provides some helpful information. If you plan on calling this, you can set
#   COPY_LOCAL_FILES_NO_AUTOSCAN to TRUE before including this file to improve cmake performance.

include(CMakeParseArguments)
include(TargetCopyLocalFiles)

# add_dependency_to_local_file_list(target dependency):
#   Given a target and a dependency which must be a defined imported target (type module or shared),
#   Adds the dependency's imported location to the target's list of REQUIRED_LOCAL_FILES, used
#   by add_local_file_copy_command. A generally internal command.
function(add_dependency_to_local_file_list target dependency)
  get_target_property(_type ${dependency} TYPE)
  get_target_property(_imported ${dependency} IMPORTED)
  if(NOT ((${_type} STREQUAL SHARED_LIBRARY OR ${_type} STREQUAL MODULE_LIBRARY) AND ${_imported}))
    message(FATAL_ERROR "add_dependency_to_local_file_list called with target ${target} on bad dependency ${dependency}")
    return()
  endif()

  #TODO: Do something with this
  get_target_property(_install_subdir ${dependency} INSTALL_SUBDIR)
  if(NOT _install_subdir)
    set(_install_subdir .)
  endif()

  get_target_property(_imported_location ${dependency} IMPORTED_LOCATION)
  get_target_property(_imported_location_debug ${dependency} IMPORTED_LOCATION_DEBUG)
  get_target_property(_imported_location_release ${dependency} IMPORTED_LOCATION_RELEASE)

  get_target_property(_is_bundle ${target} MACOSX_BUNDLE)
  if(_is_bundle )
    if(_install_subdir STREQUAL ".")
      set(_install_subdir ../Frameworks)
    endif()
  endif()

  if(_imported_location)
    add_local_files(${target} DIRECTORY ${_install_subdir} FILES ${_imported_location} )
  else()
    add_local_files(${target} DIRECTORY ${_install_subdir} DEBUG ${_imported_location_debug} RELEASE ${_imported_location_release})
  endif()
endfunction()

# scan_dependencies_for_dlls(target ...)
#  Recursively scans a target's INTERFACE_LINK_LIBRARIES and INTERFACE_LINK_MODULES properties
#  for targets, adding any shared libraries or modules IMPORTED_LOCATIONs to the target's list
#  of copied local files. Any dependency which are found (and are not generator expressions or .lib/.so files)
#  but are not targets are appended to the target's UNRESOLVED_DEPENDENCIES property for resolution on
#  a later pass. Any targets who'se UNRESOLVED_DEPENDENCIES property is non-empty is added to the global
#  list of UNRESOLVED_TARGETS
function(scan_dependencies_for_dlls target)
  foreach(dependency ${ARGN})
    set(_found FALSE)
    is_in_prop_set(TARGET ${target} RESOLVED_DEPENDENCIES ${dependency} _found)
    if(_found)
      verbose_message("Skipping already found dependency for ${target}: ${dependency}")
    else()
      if(TARGET ${dependency})
        verbose_message("Adding dependency to ${target}:${dependency}")
        add_to_prop_set(TARGET ${target} RESOLVED_DEPENDENCIES ${dependency})
        remove_from_prop_set(TARGET ${target} UNRESOLVED_DEPENDENCIES ${dependency})

        get_target_property(_type ${dependency} TYPE)
        get_target_property(_imported ${dependency} IMPORTED)

        if((${_type} STREQUAL SHARED_LIBRARY OR ${_type} STREQUAL MODULE_LIBRARY) AND ${_imported})
          add_dependency_to_local_file_list(${target} ${dependency})
        endif()

        get_target_property(_libraries ${dependency} INTERFACE_LINK_LIBRARIES)
        get_target_property(_modules ${dependency} INTERFACE_LINK_MODULES)
        if(NOT _libraries)
          set(_libraries)
        endif()
        if(NOT _modules)
          set(_modules)
        endif()

        scan_dependencies_for_dlls(${target} ${_libraries} ${_modules})
      elseif(NOT dependency MATCHES "^[$-][<-]" AND NOT dependency MATCHES ".*[.-]..?.?$")
        #Dependency is not a target and not some freaky generator expression, add to the list of unresolved.
        verbose_message("Adding unresolved dependency to ${target}:${dependency}")
        add_to_prop_set(TARGET ${target} UNRESOLVED_DEPENDENCIES ${dependency})
      endif()
    endif()
  endforeach()

  #If the target has unresolve dependencies, save it for later
  get_target_property(_target_unresolved ${target} UNRESOLVED_DEPENDENCIES)
  if(_target_unresolved)
    add_to_prop_set(GLOBAL "" UNRESOLVED_TARGETS ${target})
  endif()
endfunction()

function(target_copy_shared_libraries target)
  #early out if the target isn't an EXECUTABLE
  get_target_property(_target_type ${target} TYPE)
  if(NOT ${_target_type} STREQUAL EXECUTABLE)
    return()
  endif()

  add_local_file_copy_command(${target})

  get_target_property(_libraries ${target} INTERFACE_LINK_LIBRARIES)
  get_target_property(_modules ${target} INTERFACE_LINK_MODULES)
  if(NOT _libraries)
    set(_libraries)
  endif()
  if(NOT _modules)
    set(_modules)
  endif()

  if(NOT _libraries AND NOT _modules)
    message(WARNING "${target} does not have any defined dependencies - call target_link_libraries before you call this!")
  endif()

  scan_dependencies_for_dlls(${target} ${_libraries} ${_modules})
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
  target_copy_shared_libraries(${target})
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
  if(NOT _global_unresolved)
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

function(verify_shared_libraries_resolved)
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

if(NOT COPY_LOCAL_FILES_NO_AUTOSCAN)
  variable_watch(CMAKE_BACKWARDS_COMPATIBILITY scan_unresolved_EOFHook)
endif()
