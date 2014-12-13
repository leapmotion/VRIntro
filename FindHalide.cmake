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
    set(_link_flags "-lz")
  else()
    set(_link_flags "/STACK:8388608,1048576")
  endif()

  get_filename_component(_filepath ${generator_file} ABSOLUTE)
  
  #This isn't exactly ideal - add_custom_command would be better but I'm not sure
  #how to make it compile and run a thing 
  try_run(_run_result _compile_result ${CMAKE_BINARY_DIR} ${_filepath}
    CMAKE_FLAGS "-DCMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD=c++11 -DCMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY=libc++"
    LINK_LIBRARIES Halide::Halide ${_link_flags}
    COMPILE_DEFINITIONS ${_compile_flags}
    COMPILE_OUTPUT_VARIABLE _compile_output
    RUN_OUTPUT_VARIABLE _run_output
    ARGS ${CMAKE_BINARY_DIR}/${aot_file}
  )

  set(${sourcevar} ${${sourcevar}} ${generator_file} ${CMAKE_BINARY_DIR}/${aot_file}.h ${CMAKE_BINARY_DIR}/${aot_file}.o PARENT_SCOPE)
  set_source_files_properties( ${generator_file} PROPERTIES HEADER_FILE_ONLY TRUE)
  source_group("Halide Generators" FILES ${generator_file})
  #message("compile=${_compile_result},${_compile_output}")
  #message("run=${_run_result},${_run_output}")
endfunction()
