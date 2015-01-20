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

    list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 .lib)
    find_library(Halide_STATIC_LIB Halide HINTS "${Halide_ROOT_DIR}" PATH_SUFFIXES bin)
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
  get_filename_component(_filepath ${generator_file} ABSOLUTE)
  get_filename_component(_filename ${generator_file} NAME)
  get_filename_component(_fileroot ${_filepath} NAME_WE)

  if(NOT TARGET ${_fileroot})
    set(_compile_flags "-std=c++11" "-DBUILD_ANDROID=1")
    if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
      set(_link_flags -lpthread -lz -ldl)
    else()
      set(_link_flags -lz)
    endif()
    if(NOT BUILD_ARM)
      add_executable(${_fileroot} ${_filename})
      get_target_property(_sources ${_fileroot} SOURCES)
      set_property(TARGET ${_fileroot} PROPERTY FOLDER "Halide Generators")

      target_include_directories(${_fileroot} PUBLIC ${Halide_INCLUDE_DIR})
      target_link_libraries(${_fileroot} PUBLIC ${Halide_STATIC_LIB})
      if(NOT WIN32)
        target_link_libraries(${_fileroot} PRIVATE ${_link_flags})
      endif()
    endif()
    if(BUILD_64_BIT)
      set(_bits_flags "-DBUILD_64_BIT=1")
    endif()
    set(_compile_flags "-std=c++11" "-DBUILD_ANDROID=1" "${_bits_flags}")
    # For the cross-compilation hack, we cannot use add_definitions()
    if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
      set(_link_flags -lpthread -lz -ldl)
    else()
      set(_link_flags -lz)
    endif()
    if(WIN32)
      target_compile_options(${_fileroot} PRIVATE /MD /U_DEBUG)

      # FIXME: what's now the proper way to grab DLLs required for the build process
      file(COPY ${Halide_SHARED_LIB} DESTINATION ${PROJECT_BINARY_DIR}/bin/Release/)
      file(COPY ${Halide_SHARED_LIB} DESTINATION ${PROJECT_BINARY_DIR}/bin/Debug/)
    endif()
    if(BUILD_ARM)
      if(WIN32)
        message(FATAL_ERROR "Halide ahead-of-time cross-compilation not supported on Windows")
      endif()
      add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/bin/HalideGenerators/${_fileroot}
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
        COMMAND mkdir -p bin/HalideGenerators
        COMMAND c++ "${_filepath}" -o bin/HalideGenerators/${_fileroot} -I${Halide_INCLUDE_DIR} ${_compile_flags} ${Halide_LIBRARY} ${_link_flags}
        DEPENDS ${generator_file}
      )
    endif()
  endif()

  if(BUILD_ARM)
    set(_command "${PROJECT_BINARY_DIR}/bin/HalideGenerators/${_fileroot}")
    set(_depends "${PROJECT_BINARY_DIR}/bin/HalideGenerators/${_fileroot}")
  else()
    set(_command "$<TARGET_FILE:${_fileroot}>")
    set(_depends "${_fileroot}")
  endif()

  add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/${aot_file}.h ${PROJECT_BINARY_DIR}/${aot_file}.o
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    COMMAND "${_command}" "${aot_file}" ${ARGN}
    DEPENDS "${_depends}"
  )

  set(${sourcevar} ${${sourcevar}} ${PROJECT_BINARY_DIR}/${aot_file}.h ${PROJECT_BINARY_DIR}/${aot_file}.o PARENT_SCOPE)
  #message("run=${_run_result},${_run_output}")
endfunction()
