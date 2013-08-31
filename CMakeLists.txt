cmake_minimum_required (VERSION 2.6)

project (voxel_octrees)

set (CMAKE_C_FLAGS_RELEASE "-O2")
set (CMAKE_C_FLAGS_DEBUG "-O0 -g")
set (CMAKE_BUILD_TYPE RELEASE)

find_package (SDL REQUIRED)
include_directories (${SDL_INCLUDE_DIR})

add_subdirectory (lib)
add_subdirectory (simple-renderer)

find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
