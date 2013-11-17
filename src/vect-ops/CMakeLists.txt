cmake_minimum_required (VERSION 2.6)

# FIXME: Do not forget no-SIMD variant
if (SSE_ENABLE_SEARCH)
add_definitions(-msse3)
add_library (vect-ops SHARED vect-ops-sse.c)
else (SSE_ENABLE_SEARCH)
add_library (vect-ops SHARED vect-ops.c)
endif (SSE_ENABLE_SEARCH)
