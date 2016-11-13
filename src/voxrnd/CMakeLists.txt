cmake_minimum_required (VERSION 2.6)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/voxrnd.ld.in ${CMAKE_CURRENT_BINARY_DIR}/voxrnd.ld)
include_directories (${SDL_INCLUDE_DIR})
add_definitions (-DVOXRND_SOURCE -fblocks)

if (GCD_FOUND)
include_directories (${GCD_INCLUDE_DIR})
add_definitions (-DUSE_GCD)
endif (GCD_FOUND)

set (VOXRND_SOURCES simple-camera.c distorted-camera.c renderer.c vect-ops.c vect-ops-sse.c)
add_library (voxrnd SHARED ${VOXRND_SOURCES})
set_target_properties (voxrnd PROPERTIES VERSION ${voxvision_VERSION}
                                         SOVERSION ${voxvision_VERSION_MAJOR}
                                         LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voxrnd.ld
                                         LINK_FLAGS "-Wl,--version-script ${CMAKE_CURRENT_BINARY_DIR}/voxrnd.ld")

target_link_libraries (voxrnd ${SDL_LIBRARY} m)
if (GCD_FOUND)
target_link_libraries (voxrnd ${GCD_LIBRARY})
endif (GCD_FOUND)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
target_link_libraries (voxrnd BlocksRuntime)
endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

install (FILES camera.h renderer.h simple-camera.h distorted-camera.h
         DESTINATION include/voxvision/voxrnd)
install (TARGETS voxrnd LIBRARY
         DESTINATION lib)
