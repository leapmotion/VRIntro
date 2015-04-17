#.rst
#
# DefinePostBuildResourceCopyRules
# --------------
#
# Created by Victor Dods.
# Hides the nastiness of defining platform-specific rules for installing
# resource files post-build.

# The DESTINATION_PATH_BASE option can be used to specify an (absolute) path which
# will be used as the base path for determining where the resource directory is
# located. The resource directory will depend on the platform and other options,
# such as if the target has MACOSX_BUNDLE specified.

# The RESOURCE_PATH_BASE option can be used to specify an (absolute) path which
# will be used as the base path for where all of the resources will be copied
# from. By default, if it is not provided, the base path will be empty, which
# may be useful when the resources are already provided with absolute paths
function(define_post_build_resource_copy_rules)
    # Do the fancy map-style parsing of the arguments
    set(_options "")
    set(_one_value_args
        TARGET
        DESTINATION_PATH_BASE
        RESOURCE_PATH_BASE
    )
    set(_multi_value_args
        RESOURCES
        TARGETS
    )
    cmake_parse_arguments(_arg "${_options}" "${_one_value_args}" "${_multi_value_args}" ${ARGN})

    if(NOT _arg_TARGET)
        message(SEND_ERROR "must specify a value for TARGET in define_post_build_resource_copy_rules")
        return()
    endif()

    if(_arg_RELATIVE_PATH_BASE OR _arg_RELATIVE_PATH_RESOURCES OR _arg_ABSOLUTE_PATH_RESOURCES)
        message(SEND_ERROR "using the old argument names. Please read the documentation and update your arguments")
        return()
    endif()

    # The "base" resources dir which may be appended to, depending on the platform and other conditions.
    set(_destination_dir "${PROJECT_BINARY_DIR}")
    # Override it with the DESTINATION_PATH_BASE value if specified.
    if(_arg_DESTINATION_PATH_BASE)
        set(_destination_dir "${_arg_DESTINATION_PATH_BASE}")
    endif()

    # The "base" resources dir of where the resources are relative to
    set(_resource_base_dir "")
    # Override it with the RESOURCE_PATH_BASE value if specified.
    if(_arg_RESOURCE_PATH_BASE)
        set(_resource_base_dir "${_arg_RESOURCE_PATH_BASE}")
    endif()

    # Decide where the resources directory is on each platform.
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # This is the correct way to detect Mac OS X operating system -- see http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
        # TODO: apparently there is a different "correct" way to install files on Mac;
        # see: http://www.cmake.org/cmake/help/v3.0/prop_sf/MACOSX_PACKAGE_LOCATION.html
        # Though this seems unnecessary. Maybe we'll do this later.
        if(${CMAKE_GENERATOR} MATCHES "Xcode")
            # CMAKE_BUILD_TYPE will be one of Release, Debug, etc.
            set(_destination_dir "${_destination_dir}/${CMAKE_BUILD_TYPE}")
        endif()


        # Check to see if the target is a Mac OS X bundle. If so, set the resources dir to the appropriate
        # directory in the bundle
        get_property(_mac_bundle TARGET ${_arg_TARGET} PROPERTY MACOSX_BUNDLE SET)
        if (_mac_bundle)
            set(_destination_dir "${_destination_dir}/${_arg_TARGET}.app/Contents/Resources")
        endif()
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        # CMAKE_CFG_INTDIR  will be one of Release, Debug, etc.
        set(_destination_dir "${_destination_dir}/${CMAKE_CFG_INTDIR}")
    endif()

    # Add post-build rules for copying the resources into the correct place.
    # This should happen at the end of the build.
    foreach(_resource ${_arg_RESOURCES})
        # Add the post-build command for copying files (if different)
        add_custom_command(
            TARGET ${_arg_TARGET}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_resource_base_dir}/${_resource}" "${_destination_dir}/${_resource}"
        )
    endforeach()

    # Also add install rules for Linux
    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        install(FILES ${_arg_RESOURCES} DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif()
endfunction()
