
cmake_minimum_required (VERSION 2.6)

include_directories (${SDL_INCLUDE_DIR})
set (VOXRND_SOURCES local-loop.c renderer-ctx.c camera.c renderer.c)

if (SSE_ENABLE_SEARCH)
add_definitions(-msse3)
add_library (voxrnd SHARED vect-ops-sse.c ${VOXRND_SOURCES})
else (SSE_ENABLE_SEARCH)
add_library (voxrnd SHARED vect-ops.c ${VOXRND_SOURCES})
endif (SSE_ENABLE_SEARCH)

target_link_libraries (voxrnd ${SDL_LIBRARY} m)
