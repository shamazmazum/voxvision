cmake_minimum_required (VERSION 2.6)

project (voxvision)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules")

set (voxvision_VERSION_MAJOR 0)
set (voxvision_VERSION_MINOR 34)
set (voxvision_VERSION ${voxvision_VERSION_MAJOR}.${voxvision_VERSION_MINOR})

set (CMAKE_C_FLAGS_RELEASE "-O3 -march=native")
set (CMAKE_C_FLAGS_DEBUG "-O0 -g")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

# Dances with linux
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

option (SSE_INTRIN "Enable SSE intrinsic" ON)
option (WITH_GCD "Enable GCD" ON)
option (WITH_DTRACE "Enable Dtrace" OFF)

if (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
  set (WITH_DTRACE ON)
endif (CMAKE_BUILD_TYPE STREQUAL "DEBUG")

if (WITH_DTRACE)
  add_definitions (-DSTATISTICS)
  set (WITH_GCD OFF)
  find_package (Dtrace REQUIRED)
endif (WITH_DTRACE)

find_package (SDL2 REQUIRED)
find_package (IniParser)
find_package (Lua 5.3)
set (Python_ADDITIONAL_VERSIONS 2.7)
find_package (PythonInterp REQUIRED)
if (WITH_GCD)
  message (STATUS "Compiling with GCD. If configuration is unsuccessful, try -DWITH_GCD=OFF")
  find_package (GCD REQUIRED)
endif (WITH_GCD)
find_package (VN3D)

if (SSE_INTRIN)
  add_definitions (-msse3 -msse4.1)
endif (SSE_INTRIN)
add_definitions (-DVOXVISION_BUILDING)
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
endif(CUNIT_FOUND)

install (DIRECTORY data/ DESTINATION share/voxvision/data
  FILES_MATCHING PATTERN "*.dat")
install (DIRECTORY examples/ DESTINATION share/examples/voxvision
  FILES_MATCHING PATTERN "*.cfg" PATTERN "*.lua")
