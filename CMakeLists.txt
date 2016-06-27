cmake_minimum_required (VERSION 2.6)

project (voxvision)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules")

set (voxvision_VERSION_MAJOR 0)
set (voxvision_VERSION_MINOR 8)
set (voxvision_VERSION ${voxvision_VERSION_MAJOR}.${voxvision_VERSION_MINOR})

set (CMAKE_C_FLAGS_RELEASE "-O2")
set (CMAKE_C_FLAGS_DEBUG "-O0 -g")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# Dances with linux
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

find_package (SDL REQUIRED)
find_package (IniParser)
find_package (GCD)

if (NOT GCD_FOUND)
message (WARNING "Could not find GCD. voxrnd will still be working, but slow.
   If you are using FreeBSD, consider installing devel/libdispatch
   and devel/compiler-rt ports.")
endif (NOT GCD_FOUND)

option (SSE_INTRIN "Enable SSE intrinsic")
if (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
add_definitions (-DSTATISTICS)
unset (GCD_FOUND)
endif (CMAKE_BUILD_TYPE STREQUAL "DEBUG")

if (SSE_INTRIN)
add_definitions (-msse3 -msse4.1)
endif (SSE_INTRIN)
add_subdirectory (src)
add_subdirectory (benchmarks)

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
add_custom_target (check
run-tests
COMMENT "Run tests"
)
endif(CUNIT_FOUND)

install (DIRECTORY examples/ DESTINATION share/examples/voxvision
         FILES_MATCHING PATTERN "*.cfg" PATTERN "*.dat")
