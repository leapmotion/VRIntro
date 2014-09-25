# Root dependency
find_package(Components REQUIRED)

# This directory is a basis for all other includes
include_directories(.)

# All libraries for this app
add_subdirectory(VRIntro)
add_subdirectory(VRIntroLib)