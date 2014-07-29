#.rst
# Components
# ------------
#
# Created by Victor Dods
# Functions and macros which assist in defining "components".
#
# A "component" can be thought of as a "sub-library" (in the sense that it is a small library and
# is subordinate to the whole library).  A component satisfies two requirements:
# - Has a well-defined purpose, scope, and feature set.
# - Has well-defined dependencies, which are explicitly declared and are minimal.
#
# This is the list of defined components.  The component name should be identical to
# the subdirectory which contains all its source files.  A component name should be
# a C identifier that is WordCapitalized.  Each component should have
# the following macros defined.
#   Foo_SOURCES                         The list of source files for component "Foo"
#   Foo_INSTALL_FILES                   The list of files to copy into a "release" archive.
#   Foo_EXPLICIT_COMPONENT_DEPENDENCIES The list of components which component "Foo" explicitly depends on; not,
#                                       for example, components which "Foo" depends on through other components.
#                                       It's ok if there are redundancies here.
#   Foo_EXPLICIT_LIBRARY_DEPENDENCIES   The list of library-version pairs which component "Foo" explicity depends
#                                       on; not, for example, libraries which "Foo" depends on through other
#                                       components or libraries.  It's ok if there are redundancies here.
# The total component dependencies of a component can be determined recursively using these
# definitions.  Similarly, the total library dependencies of a component can be determined.

# Design notes for components
# ---------------------------
# - Each component should be able to be compiled as a library, thereby enforcing the strict
#   modularization which defines it.  If a component were compiled into a library, its component
#   dependencies would have to be either
#   * Compiled in, or
#   * linked as libraries themselves.
# - A compiled-and-minimal distribution of a component or set of components should be layed out as follows:
#   * DIST_DIR/include/                                 # This is the include dir for everything
#   * DIST_DIR/include/Component1/something.h
#   * DIST_DIR/include/Component1/otherthing.h
#   * DIST_DIR/include/Component2/foo.h
#   * DIST_DIR/lib/                                     # Dir for all the lib files (static and dynamic)
#   * DIST_DIR/lib/libComponent1.a
#   * DIST_DIR/lib/libComponent1.so
#   * DIST_DIR/lib/libComponent2.a
#   * DIST_DIR/lib/libComponent2.so
#   * DIST_DIR/resources/                               # Dir for non-compiled, non-header files, e.g. shaders.
#   * DIST_DIR/resources/fancy-shader.glsl
#   * DIST_DIR/resources/pumpkin.png
#   * DIST_DIR/cmake-modules/                           # Contains files for easy/correct usage of cmake
#   * DIST_DIR/cmake-modules/Components.cmake           # Components cmake functionality
#   * DIST_DIR/cmake-modules/FindComponents.cmake       # Defines how to include and link the lib
# - The components library should also be usable from its source dir, rather than only from
#   a packaged distribution.  Because in theory this should only be done by Components team members,
#   it may be acceptable to manually configure the include/lib/cmake-modules dir variables in ccmake.

# COMPONENTS is a list of strings enumerating all the defined components.  Defining a component
# should append to it -- the define_component macro is a convience macro for defining all the
# correct variables for a component.
macro(begin_component_definitions)
    set(COMPONENTS "")
endmacro()

macro(define_component NAME SOURCES_TO_INSTALL SOURCES_NO_INSTALL RESOURCES_TO_INSTALL EXPLICIT_COMPONENT_DEPENDENCIES EXPLICIT_LIBRARY_DEPENDENCIES)
    # message("defining component \"${NAME}\" with:\n"
    #         "\tSOURCES_TO_INSTALL = ${SOURCES_TO_INSTALL}\n"
    #         "\tSOURCES_NO_INSTALL = ${SOURCES_NO_INSTALL}\n"
    #         "\tRESOURCES_TO_INSTALL = ${RESOURCES_TO_INSTALL}\n"
    #         "\tEXPLICIT_COMPONENT_DEPENDENCIES = ${EXPLICIT_COMPONENT_DEPENDENCIES}\n"
    #         "\tEXPLICIT_LIBRARY_DEPENDENCIES = ${EXPLICIT_LIBRARY_DEPENDENCIES}")
    set(COMPONENTS ${COMPONENTS} ${NAME})
    set(${NAME}_SOURCES ${SOURCES_TO_INSTALL} ${SOURCES_NO_INSTALL})
    set(${NAME}_SOURCES_TO_INSTALL ${SOURCES_TO_INSTALL})
    set(${NAME}_RESOURCES_TO_INSTALL ${RESOURCES_TO_INSTALL})
    set(${NAME}_EXPLICIT_COMPONENT_DEPENDENCIES ${EXPLICIT_COMPONENT_DEPENDENCIES})
    set(${NAME}_EXPLICIT_LIBRARY_DEPENDENCIES ${EXPLICIT_LIBRARY_DEPENDENCIES})
    # message("defined cmake variables:\n"
    #         "\t${NAME}_SOURCES = ${${NAME}_SOURCES}\n"
    #         "\t${NAME}_SOURCES_TO_INSTALL = ${${NAME}_SOURCES_TO_INSTALL}\n"
    #         "\t${NAME}_RESOURCES_TO_INSTALL = ${${NAME}_RESOURCES_TO_INSTALL}\n"
    #         "\t${NAME}_EXPLICIT_COMPONENT_DEPENDENCIES = ${${NAME}_EXPLICIT_COMPONENT_DEPENDENCIES}\n"
    #         "\t${NAME}_EXPLICIT_LIBRARY_DEPENDENCIES = ${${NAME}_EXPLICIT_LIBRARY_DEPENDENCIES}\n")
endmacro()

# # TODO MAKE THIS FUNCTION WORK
# function(determine_include_directories DESIRED_COMPONENTS INCLUDE_DIRECTORIES)
#     set(${INCLUDE_DIRECTORIES} "")
#     foreach(COMPONENT ${DESIRED_COMPONENTS})
#         #list(APPEND ${INCLUDE_DIRECTORIES} include/${COMPONENT})
#         set(${INCLUDE_DIRECTORIES} ${${INCLUDE_DIRECTORIES}} include/${COMPONENT})
#     endforeach()
#     message("hippo INCLUDE_DIRECTORIES = ${INCLUDE_DIRECTORIES}")
# endfunction()

function(define_install_rules TARGET DESIRED_COMPONENTS)
    # message("defining install rule for target ${TARGET}")
    set(INCLUDE_DIRECTORIES "")
    foreach(COMPONENT ${DESIRED_COMPONENTS}) # This loop should be replaced with determine_include_directories
        list(APPEND INCLUDE_DIRECTORIES include/${COMPONENT})
    endforeach()
    install(
        TARGETS ${TARGET}
        EXPORT ${TARGET}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION ${INCLUDE_DIRECTORIES})
    install(
        EXPORT ${TARGET}
        DESTINATION .
        FILE ComponentsConfig.cmake)
    # message("defining install rules for components ${DESIRED_COMPONENTS}")
    foreach(COMPONENT ${DESIRED_COMPONENTS})
        # message("defining install rules for component ${COMPONENT}")
        foreach(SOURCE_TO_INSTALL ${${COMPONENT}_SOURCES_TO_INSTALL})
            # message("\tdefining include file install rule ${COMPONENT}/${SOURCE_TO_INSTALL} -> include/${COMPONENT}/${SOURCE_TO_INSTALL}")
            install(
                FILES ${COMPONENT}/${SOURCE_TO_INSTALL}
                DESTINATION include/${COMPONENT}
            )
        endforeach()
        foreach(RESOURCE_TO_INSTALL ${${COMPONENT}_RESOURCES_TO_INSTALL})
            # message("\tdefining resource file install rule ${COMPONENT}/${RESOURCE_TO_INSTALL} -> resources/${RESOURCE_TO_INSTALL}")
            install(
                FILES ${COMPONENT}/${RESOURCE_TO_INSTALL}
                DESTINATION resources
            )
        endforeach()
    endforeach()
endfunction()
