cmake_minimum_required (VERSION 2.6)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/voxrnd.ld.in ${CMAKE_CURRENT_BINARY_DIR}/voxrnd.ld)
include_directories (${SDL2_INCLUDE_DIR})
add_definitions (-DVOXRND_SOURCE)

if (GCD_FOUND)
include_directories (${GCD_INCLUDE_DIR})
add_definitions (-fblocks -DUSE_GCD)
endif (GCD_FOUND)

if (0)
set (VOXRND_SOURCES camera.c renderer.c)
if (SSE_INTRIN)
add_definitions(-msse3)
set (VOXRND_SOURCES ${VOXRND_SOURCES} vect-ops-sse.c)
else (SSE_INTRIN)
set (VOXRND_SOURCES ${VOXRND_SOURCES} vect-ops.c)
endif (SSE_INTRIN)
else (0)
set (VOXRND_SOURCES camera.c renderer.c vect-ops.c)
endif (0)

add_library (voxrnd SHARED ${VOXRND_SOURCES})
set_target_properties (voxrnd PROPERTIES VERSION ${voxvision_VERSION}
                                         SOVERSION ${voxvision_VERSION_MAJOR}
                                         LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voxrnd.ld
                                         LINK_FLAGS "-Wl,--version-script ${CMAKE_CURRENT_BINARY_DIR}/voxrnd.ld")

target_link_libraries (voxrnd ${SDL2_LIBRARY} m)
if (GCD_FOUND)
target_link_libraries (voxrnd ${GCD_LIBRARY})
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
target_link_libraries (voxrnd BlocksRuntime)
endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
endif (GCD_FOUND)

install (FILES camera.h renderer.h
         DESTINATION include/voxvision/voxrnd)
install (TARGETS voxrnd LIBRARY
         DESTINATION lib)
