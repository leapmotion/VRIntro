include(Common)

set(VRIntro_SOURCES
    main.cpp
    APIFrameSupplier.h
    APIFrameSupplier.cpp
    LeapListener.cpp
    LeapListener.h
)

option(SHOW_CONSOLE "Display console window for debugging" OFF)

if(NOT SHOW_CONSOLE AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    set(_add_executable_options WIN32)
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_DEBUG_FLAGS} /Zi /fp:fast /MP")
    set(CMAKE_CXX_RELEASE_FLAGS "${CMAKE_CXX_RELEASE_FLAGS} /fp:except /MP")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT /ignore:4099")
endif()

ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" VRIntro_SOURCES)
add_executable(VRIntro ${_add_executable_options} ${VRIntro_SOURCES})
target_link_libraries(VRIntro VRIntroLib)
target_package(VRIntro Leap)

###################################################################################################
# Resource rules
###################################################################################################

include(DefinePostBuildResourceCopyRules)
define_post_build_resource_copy_rules(
    TARGET
        VRIntro
    RELATIVE_PATH_BASE
        $<TARGET_FILE_DIR:VRIntro>/..
    RELATIVE_PATH_RESOURCES
        shaders/fractal-frag.glsl
        shaders/fractal-vert.glsl
        shaders/passthrough-frag.glsl
        shaders/passthrough-vert.glsl
        shaders/solid-frag.glsl
        shaders/solid-vert.glsl
        shaders/transparent-frag.glsl
        shaders/transparent-vert.glsl
        images/help.png
        images/lowfps.png
        images/no_oculus.png
        images/level3_popup.png
        images/level4_popup.png
        images/no_images.png
        images/random.png
        RunMirrored.bat
    ABSOLUTE_PATH_RESOURCES
        ${Components_DIR}/resources/material-frag.glsl
        ${Components_DIR}/resources/matrix-transformed-vert.glsl
)
