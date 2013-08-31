cmake_minimum_required (VERSION 2.6)

project (voxel_octrees)

set (CMAKE_C_FLAGS_RELEASE "-O2")
set (CMAKE_C_FLAGS_DEBUG "-O0 -g")
set (CMAKE_BUILD_TYPE RELEASE)

find_package (SDL REQUIRED)
include_directories (${SDL_INCLUDE_DIR})

add_subdirectory (lib)
add_subdirectory (simple-renderer)
