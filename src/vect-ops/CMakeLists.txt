
cmake_minimum_required (VERSION 2.6)

include_directories (${SDL_INCLUDE_DIR})
set (VECT_OPS_SOURCES local-loop.c renderer-ctx.c camera.c)

if (SSE_ENABLE_SEARCH)
add_definitions(-msse3)
add_library (vect-ops SHARED vect-ops-sse.c ${VECT_OPS_SOURCES})
else (SSE_ENABLE_SEARCH)
add_library (vect-ops SHARED vect-ops.c ${VECT_OPS_SOURCES})
endif (SSE_ENABLE_SEARCH)

target_link_libraries (vect-ops m)
