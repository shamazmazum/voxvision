cmake_minimum_required (VERSION 2.6)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/voxrnd.ld.in ${CMAKE_CURRENT_BINARY_DIR}/voxrnd.ld)
include_directories (${SDL_INCLUDE_DIR})
set (VOXRND_SOURCES local-loop.c camera.c renderer.c)
add_definitions (-DVOXRND_SOURCE)

if (SSE_INTRIN)
add_definitions(-msse3)
add_library (voxrnd SHARED vect-ops-sse.c ${VOXRND_SOURCES})
else (SSE_INTRIN)
add_library (voxrnd SHARED vect-ops.c ${VOXRND_SOURCES})
endif (SSE_INTRIN)

set_target_properties (voxrnd PROPERTIES VERSION ${voxvision_VERSION}
                                         SOVERSION ${voxvision_VERSION_MAJOR}
                                         LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voxrnd.ld
                                         LINK_FLAGS "-Wl,--version-script ${CMAKE_CURRENT_BINARY_DIR}/voxrnd.ld")

target_link_libraries (voxrnd ${SDL_LIBRARY} m)
