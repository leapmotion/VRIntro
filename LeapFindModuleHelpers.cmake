#.rst
#LeapFindModuleHelpers

function(find_multitype_library shared_out static_out import_out)
  list(REMOVE_AT ARGV 0) #remove shared_out
  list(REMOVE_AT ARGV 0) #remove static_out
  list(REMOVE_AT ARGV 0) #remove import_out
  
  set(_oldlibsuffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})
    find_library(${shared_out} ${ARGV})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
    find_library(${static_out} ${ARGV})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_IMPORT_LIBRARY_SUFFIX})
    find_library(${import_out} ${ARGV})
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${_oldlibsuffixes})

  #TODO:verify the types of the static & import libraries
  if(MSVC)
    #file(READ ${static_out})
  endif()

endfunction()

#Checks <namespace>_SHARED_LIB, <namespace>_STATIC_LIB and <namespace>_IMPORT_LIB
#And fills <namespace>_LIBRARY with the appropriate lib type depending on which is found
#If both are found, it will default to shared.  We may, at a later time, also add verification
#of if the static library is actually a static lib and not an import lib on windows.
#It will then fill <namespace>_LIBRARY_TYPE with either SHARED or STATIC
function(select_library_type namespace)
  #select the primary library type
  if(${namespace}_SHARED_LIB AND EXISTS "${${namespace}_SHARED_LIB}")
    
    #add either the .lib or the .dylib to the libraries list
    if(${namespace}_IMPORT_LIB AND EXISTS "${${namespace}_IMPORT_LIB}")
      set(${namespace}_LIBRARIES "${${namespace}_LIBRARIES}" "${${namespace}_IMPORT_LIB}" PARENT_SCOPE)
    else()
      set(${namespace}_LIBRARIES "${${namespace}_LIBRARIES}" "${${namespace}_SHARED_LIB}" PARENT_SCOPE)
    endif()

    set(${namespace}_LIBRARY "${${namespace}_SHARED_LIB}" PARENT_SCOPE)
    set(${namespace}_LIBRARY_TYPE "SHARED" PARENT_SCOPE)
  elseif(${namespace}_STATIC_LIB AND EXISTS "${${namespace}_STATIC_LIB}")
    set(${namespace}_LIBRARIES "${${namespace}_LIBRARIES}" "${${namespace}_STATIC_LIB}" PARENT_SCOPE)
    set(${namespace}_LIBRARY "${${namespace}_STATIC_LIB}" PARENT_SCOPE)
    set(${namespace}_LIBRARY_TYPE "STATIC" PARENT_SCOPE)
  endif()

endfunction()

function(find_likely_folders package folder_list_var path_list )
  list(REMOVE_AT ARGV 0) #pop package name
  list(REMOVE_AT ARGV 0) #pop folder_list_var

  foreach(_path ${ARGV})
    file(GLOB _subdirs RELATIVE ${_path} ${_path}/*)
    foreach(_subdir ${_subdirs})
      if(IS_DIRECTORY ${_path}/${_subdir} AND _subdir MATCHES "${package}.*")
        list(APPEND _folders ${_path}/${_subdir})
      endif()
    endforeach()
  endforeach()

  set(${folder_list_var} "${_folders}" PARENT_SCOPE)
endfunction()
