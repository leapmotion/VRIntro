#.rst:
# FindSDL
# -------
#
# Locate SDL library.  Modified by Walter Gray to be compatible with SDL 2.x and
# provide import library pseudo targets. Untested with SDL 1.x.
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#   SDL::SDL
#     Basic import target.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
# This module defines the following variables
#
#   SDL_ROOT_DIR
#
#   SDL_FOUND
#
#   SDL_INCLUDE_DIR
#
#   SDL_LIBRARY
#     Where to find SDL.dll/dylib if linking dynamically, or .lib/.a statically
#   SDL_IMPORT_LIB
#     Where to find SDL.lib (Windows only)
#   SDL_LIBRARIES
#     Where to find SDLmain.lib
#   SDL_VERSION_STRING
#     A human-readable string containing the version of SDL
#
# This module responds to the flag:
#
# ::
#
#   SDL_BUILDING_LIBRARY
#     If this is defined, then no SDL_main will be linked in because
#     only applications need main().
#     Otherwise, it is assumed you are building an application and this
#     module will attempt to locate and set the proper link flags
#     as part of the returned SDL_LIBRARY variable.
#
#
#
# Don't forget to include SDLmain.h and SDLmain.m your project for the
# OS X framework based version.  (Other versions link to -lSDLmain which
# this module will try to find on your behalf.) Also for OS X, this
# module will automatically add the -framework Cocoa on your behalf.
#
#
#
# Additional Note: If you see an empty SDL_LIBRARY_TEMP in your
# configuration and no SDL_LIBRARY, it means CMake did not find your SDL
# library (SDL.dll, libsdl.so, SDL.framework, etc).  Set
# SDL_LIBRARY_TEMP to point to your SDL library, and configure again.
# Similarly, if you see an empty SDL_MAIN_LIBRARY, you should set this
# value as appropriate.  These values are used to generate the final
# SDL_LIBRARY variable, but when these values are unset, SDL_LIBRARY
# does not get created.
#
#
#
# $SDLDIR is an environment variable that would correspond to the
# ./configure --prefix=$SDLDIR used in building SDL.  l.e.galup 9-20-02
#
# Modified by Eric Wing.  Added code to assist with automated building
# by using environmental variables and providing a more
# controlled/consistent search behavior.  Added new modifications to
# recognize OS X frameworks and additional Unix paths (FreeBSD, etc).
# Also corrected the header search path to follow "proper" SDL
# guidelines.  Added a search for SDLmain which is needed by some
# platforms.  Added a search for threads which is needed by some
# platforms.  Added needed compile switches for MinGW.
#
# Modified by Walter Gray. Added code to find SDL2, and create an import
# target.
#
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of SDL_LIBRARY to
# override this selection or set the CMake environment
# CMAKE_INCLUDE_PATH to modify the search paths.
#
# Note that the header path has changed from SDL/SDL.h to just SDL.h
# This needed to change because "proper" SDL convention is #include
# "SDL.h", not <SDL/SDL.h>.  This is done for portability reasons
# because not all systems place things in SDL/ (see FreeBSD).

#=============================================================================
# Copyright 2003-2009 Kitware, Inc.
# Copyright 2012 Benjamin Eikel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

#set(_likely_folders SDL SDL12 SDL11 SDL2)
set(_likely_folders )

if(NOT SDL_ROOT_DIR)
  set(_major_range RANGE 2 0)
  set(_minor_range RANGE 4 0)
  set(_patch_range RANGE 5 0)

  if(${SDL_FIND_VERSION_COUNT} GREATER 0)
    set(_major_range ${SDL_FIND_VERSION_MAJOR})
  endif()

  if(${SDL_FIND_VERSION_COUNT} GREATER 1)
    set(_minor_range ${SDL_FIND_VERSION_MINOR})
  endif()

  if(${SDL_FIND_VERSION_COUNT} GREATER 2)
    set(_patch_range ${SDL_FIND_VERSION_PATCH})
  endif()

  foreach(_major ${_major_range})
    foreach(_minor ${_minor_range})
      foreach(_patch ${_patch_range})
        if(_major GREATER 1)
          set(_version_suffix ${_major})
        endif()
        list(APPEND _likely_folders SDL${_version_suffix}-${_major}.${_minor}.${_patch})
      endforeach()
    endforeach()
  endforeach()
  list(APPEND _likely_folders SDL SDL2)
      
  find_path(SDL_ROOT_DIR 
    NAMES include/SDL_version.h
          include/SDL/SDL_version.h
          include/SDL2/SDL_version.h
    HINTS $ENV{SDLDIR}
    PATH_SUFFIXES ${_likely_folders}
  )

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(VC_LIB_PATH_SUFFIX lib/x64)
  else()
    set(VC_LIB_PATH_SUFFIX lib/x86)
  endif()

else()
  # temp hack for Mac builds
  if(NOT MSVC)
    set(SDL_LIBRARY_TEMP ${SDL_ROOT_DIR}/lib/libSDL2.a)
  endif()
endif()

find_path(SDL_INCLUDE_DIR SDL.h
    HINTS
      $ENV{SDLDIR}
      ${SDL_ROOT_DIR}
    PATH_SUFFIXES
      include
      include/SDL
      include/SDL2
    NO_DEFAULT_PATH
    NO_CMAKE_ENVIRONMENT_PATH
    NO_CMAKE_PATH
    NO_SYSTEM_ENVIRONMENT_PATH
    NO_CMAKE_SYSTEM_PATH
    NO_CMAKE_FIND_ROOT_PATH
)
  
if(SDL_INCLUDE_DIR AND EXISTS "${SDL_INCLUDE_DIR}/SDL_version.h")
  file(STRINGS "${SDL_INCLUDE_DIR}/SDL_version.h" _major_line REGEX "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_INCLUDE_DIR}/SDL_version.h" _minor_line REGEX "^#define[ \t]+SDL_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL_INCLUDE_DIR}/SDL_version.h" _patch_line REGEX "^#define[ \t]+SDL_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_VERSION_MAJOR "${_major_line}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_VERSION_MINOR "${_minor_line}")
  string(REGEX REPLACE "^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL_VERSION_PATCH "${_patch_line}")
  set(SDL_VERSION_STRING ${SDL_VERSION_MAJOR}.${SDL_VERSION_MINOR}.${SDL_VERSION_PATCH})
  unset(_major_line)
  unset(_minor_line)
  unset(_patch_line)
endif()

# SDL-1.1 is the name used by FreeBSD ports...
# don't confuse it for the version number.
find_library(SDL_CORE_LIB
  NAMES SDL SDL-1.1 SDL${SDL_VERSION_MAJOR}
  HINTS
    $ENV{SDLDIR}
    ${SDL_ROOT_DIR}
  PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
)
set(SDL_LIBRARY_TEMP ${SDL_CORE_LIB})

if(MSVC)
  set(SDL_IMPORT_LIB ${SDL_CORE_LIB})
  find_file(SDL_LIBRARY NAMES SDL.dll SDL-1.1.dll SDL2.dll HINTS "${SDL_ROOT_DIR}/lib")
else()
  set(SDL_LIBRARY ${SDL_CORE_LIB})
endif()

if(NOT SDL_BUILDING_LIBRARY)
  if(NOT ${SDL_INCLUDE_DIR} MATCHES ".framework")
    # Non-OS X framework versions expect you to also dynamically link to
    # SDLmain. This is mainly for Windows and OS X. Other (Unix) platforms
    # seem to provide SDLmain for compatibility even though they don't
    # necessarily need it.
    find_library(SDL_MAIN_LIBRARY
      NAMES SDL${SDL_VERSION_MAJOR}main
      HINTS
        $ENV{SDLDIR}
        ${SDL_ROOT_DIR}
      PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
      PATHS
      /sw
      /opt/local
      /opt/csw
      /opt
    )

  endif()
endif()

# SDL may require threads on your system.
# The Apple build may not need an explicit flag because one of the
# frameworks may already provide it.
# But for non-OSX systems, I will use the CMake Threads package.
if(NOT APPLE)
  find_package(Threads)
endif()

# MinGW needs an additional library, mwindows
# It's total link flags should look like -lmingw32 -lSDLmain -lSDL -lmwindows
# (Actually on second look, I think it only needs one of the m* libraries.)
if(MINGW)
  set(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
endif()

if(SDL_LIBRARY_TEMP)
  # For SDLmain
  if(SDL_MAIN_LIBRARY AND NOT SDL_BUILDING_LIBRARY)
    list(FIND SDL_LIBRARY_TEMP "${SDL_MAIN_LIBRARY}" _SDL_MAIN_INDEX)
    if(_SDL_MAIN_INDEX EQUAL -1)
      set(SDL_LIBRARY_TEMP "${SDL_MAIN_LIBRARY}" ${SDL_LIBRARY_TEMP})
    endif()
    unset(_SDL_MAIN_INDEX)
  endif()

  set(SDL_INTERFACE_LIBS ${SDL_MAIN_LIBRARY})

  # For OS X, SDL uses Cocoa as a backend so it must link to Cocoa (as
  # well as the dependencies of Cocoa (the frameworks: Carbon, IOKit,
  # and the library: iconv)).  CMake doesn't display the -framework Cocoa
  # string in the UI even though it actually is there if I modify a 
  # pre-used variable.  I think it has something to do with the CACHE
  # STRING.  So I use a temporary variable until the end so I can set
  # the "real" variable in one-shot.
  if(APPLE)
    set(SDL_LIBRARY_TEMP ${SDL_LIBRARY_TEMP})
    set(SDL_INTERFACE_LIBS ${SDL_INTERFACE_LIBS} "-framework Cocoa" "-framework IOKit"  "-framework Carbon" "iconv")
  endif()

  # For threads, as mentioned Apple doesn't need this.
  # In fact, there seems to be a problem if I used the Threads package
  # and try using this line, so I'm just skipping it entirely for OS X.
  if(NOT APPLE)
    set(SDL_LIBRARY_TEMP ${SDL_LIBRARY_TEMP} ${CMAKE_THREAD_LIBS_INIT})
  endif()

  # For MinGW library
  if(MINGW)
    set(SDL_LIBRARY_TEMP ${MINGW32_LIBRARY} ${SDL_LIBRARY_TEMP})
  endif()

  # Set the final string here so the GUI reflects the final state.
  set(SDL_LIBRARIES ${SDL_LIBRARY_TEMP} CACHE STRING "Where the SDL Library can be found")

endif()

find_package_handle_standard_args(SDL
                                  REQUIRED_VARS SDL_LIBRARIES SDL_CORE_LIB SDL_INCLUDE_DIR
                                  VERSION_VAR SDL_VERSION_STRING)

mark_as_advanced(SDL_LIBRARIES SDL_IMPORT_LIB SDL_INCLUDE_DIR SDL_DYNAMIC_LIB SDL_CORE_LIB)

include(CreateImportTargetHelpers)
string(REGEX MATCH "${CMAKE_STATIC_LIBRARY_SUFFIX}$" _static ${SDL_LIBRARY})
string(REGEX MATCH "${CMAKE_SHARED_LIBRARY_SUFFIX}$" _shared ${SDL_LIBRARY})

if(_static)
  generate_import_target(SDL STATIC)
elseif(_shared)
  generate_import_target(SDL SHARED)
else()
  message(FATAL_ERROR "Unable to determine library type of file ${SDL_LIBRARY}")
endif()

#HACK FOR MAC X11 DEPENDENCY
#TODO - Create a modernized FindX11.cmake module, make SDL depend on it on macs
if(APPLE)
  find_package(X11 REQUIRED)
  set_property(TARGET SDL::SDL APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${X11_INCLUDE_DIR})
endif()
