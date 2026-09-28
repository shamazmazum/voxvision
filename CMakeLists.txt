cmake_minimum_required (VERSION 3.10)

project (voxvision)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Modules")

set (voxvision_VERSION_MAJOR 0)
set (voxvision_VERSION_MINOR 34)
set (voxvision_VERSION ${voxvision_VERSION_MAJOR}.${voxvision_VERSION_MINOR})

set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")

# Dances with linux
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

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

find_package (PkgConfig REQUIRED)
find_package (SDL2 CONFIG REQUIRED)
find_package (IniParser CONFIG REQUIRED)
find_package (Lua 5.3)
if (WITH_GCD)
  message (STATUS "Compiling with GCD. If configuration is unsuccessful, try -DWITH_GCD=OFF")
  pkg_check_modules (GCD REQUIRED IMPORTED_TARGET libdispatch)
endif (WITH_GCD)
find_package (VN3D REQUIRED)

add_definitions (-DVOXVISION_BUILDING)
add_subdirectory (src)
#add_subdirectory (benchmarks)

find_package(Doxygen)
if(DOXYGEN_FOUND)
  add_subdirectory (doc)
endif(DOXYGEN_FOUND)

pkg_check_modules (CUnit IMPORTED_TARGET cunit)
if(CUnit_FOUND)
  add_subdirectory (tests)
endif(CUnit_FOUND)

install (DIRECTORY data/ DESTINATION share/voxvision/data
  FILES_MATCHING PATTERN "*.dat")
install (DIRECTORY examples/ DESTINATION share/examples/voxvision
  FILES_MATCHING PATTERN "*.cfg" PATTERN "*.lua")
