#.rst
# FindHalide
# ------------
#
# Created by Walter Gray
# Locate and configure Halide.
# On win32, it will be configured as a dynamic library since that's how it's
# distributed.
#
# Helper Functions
# ^^^^^^^^^^^^^^^^
# add_halide_generator(sourcevar generator_file aot_file_root) 
#  Given a .cpp file with a main function that takes as an argument the name of the 
#  Halide AoT .h/.o file pair to generate, compiles and runs said program and outputs
#  the file pair to the current CMAKE_BINARY_DIR. It then appends the full path to
#  aot_file_root.h and aot_file_root.o to sourcevar.
#  
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   Halide::Halide
#
# Variables
# ^^^^^^^^^
#   Halide_ROOT_DIR
#   Halide_FOUND
#   Halide_INCLUDE_DIR
#   Halide_LIBRARY
#   Halide_STATIC_LIB
#   Halide_SHARED_LIB
#   Halide_IMPORT_LIB
#   Halide_LIBRARY_TYPE

find_path(Halide_ROOT_DIR
          NAMES include/Halide.h
          PATH_SUFFIXES halide)

find_path( Halide_INCLUDE_DIR
           NAMES Halide.h
           HINTS ${Halide_ROOT_DIR}
           PATH_SUFFIXES include
           NO_DEFAULT_PATH
)

if(WIN32)
  set(Halide_LIBRARY_TYPE SHARED)
else()
  set(Halide_LIBRARY_TYPE STATIC)
endif()

if(${Halide_LIBRARY_TYPE} STREQUAL STATIC)
  list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .a)
  find_library(Halide_STATIC_LIB Halide HINTS "${Halide_ROOT_DIR}" PATH_SUFFIXES bin)
  list(REMOVE_AT CMAKE_FIND_LIBRARY_SUFFIXES 0)
  mark_as_advanced(Halide_STATIC_LIB)
  set(Halide_LIBRARY ${Halide_STATIC_LIB})
else()
  if(WIN32)
    find_library(Halide_IMPORT_LIB Halide HINTS "${Halide_ROOT_DIR}" PATH_SUFFIXES bin)

    list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .dll)
    find_library(Halide_SHARED_LIB Halide HINTS "${Halide_ROOT_DIR}" PATH_SUFFIXES bin)
    list(REMOVE_AT CMAKE_FIND_LIBRARY_SUFFIXES 0)
  else()
    message(FATAL_ERROR "Halide find module not implemented for shared config on non-win32 platforms")
  endif()

  mark_as_advanced(Halide_SHARED_LIB Halide_IMPORT_LIB)
  set(Halide_LIBRARY ${Halide_SHARED_LIB})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Halide DEFAULT_MSG Halide_ROOT_DIR Halide_INCLUDE_DIR Halide_LIBRARY)

include(CreateImportTargetHelpers)
generate_import_target(Halide ${Halide_LIBRARY_TYPE})

function(add_halide_generator sourcevar generator_file aot_file)
  if(NOT WIN32)
    set(_compile_flags "-std=c++11")
    if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
      set(_link_flags -lpthread -lz -ldl)
    else()
      set(_link_flags -lz)
    endif()
  else()
    set(_link_flags "/STACK:8388608,1048576")
  endif()

  get_filename_component(_filepath ${generator_file} ABSOLUTE)
  get_filename_component(_fileroot ${_filepath} NAME_WE)

  if(NOT WIN32)
    #TODO:Replace this with add_custom_command
    execute_process(
      COMMAND mkdir -p HalideGenerators
      COMMAND clang++ "${_filepath}" -o HalideGenerators/${_fileroot} -I${Halide_INCLUDE_DIR} ${_compile_flags} ${Halide_LIBRARY} ${_link_flags}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      RESULT_VARIABLE _result
      OUTPUT_VARIABLE _output
      ERROR_VARIABLE _error
    )
    execute_process(
      COMMAND "HalideGenerators/${_fileroot}" "${aot_file}"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )  
  else()
    message(WARNING "add_halide_generator not currently defined for this platform")
  endif()


  set(${sourcevar} ${${sourcevar}} ${generator_file} ${CMAKE_BINARY_DIR}/${aot_file}.h ${CMAKE_BINARY_DIR}/${aot_file}.o PARENT_SCOPE)
  set_source_files_properties( ${generator_file} PROPERTIES HEADER_FILE_ONLY TRUE)
  source_group("Halide Generators" FILES ${generator_file})
  #message("res=${_result},out=${_output},err=${_error}")
endfunction()
