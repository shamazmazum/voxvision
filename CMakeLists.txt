cmake_minimum_required (VERSION 2.6)

project (voxvision)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules")

set (voxvision_VERSION_MAJOR 0)
set (voxvision_VERSION_MINOR 5)

set (CMAKE_C_FLAGS_RELEASE "-O2")
set (CMAKE_C_FLAGS_DEBUG "-O0 -g")

#find_package (GC REQUIRED)
#include_directories (${BOEHM_GC_INCLUDE_DIR})

option (SSE_ENABLE "Enable SSE instructions everywhere" OFF)
option (SSE_ENABLE_CONS "Enable SSE instructions while tree construction" OFF)
option (SSE_ENABLE_SEARCH "Enable SSE instructions while searching" OFF)

if (SSE_ENABLE)
set (SSE_ENABLE_CONS ON)
set (SSE_ENABLE_SEARCH ON)
endif (SSE_ENABLE)

add_subdirectory (src)

find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)

find_package (CUnit)
if(CUNIT_FOUND)
add_subdirectory (tests)
add_custom_target (test
run-tests
COMMENT "Run tests"
)
endif(CUNIT_FOUND)
