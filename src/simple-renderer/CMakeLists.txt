cmake_minimum_required (VERSION 2.6)

find_package (SDL REQUIRED)
include_directories (${SDL_INCLUDE_DIR})

if (SSE_ENABLE_CONS OR SSE_ENABLE_SEARCH)
add_executable (test_renderer main.c renderer.c skull_sse.o xmas_tree_sse.o) 
else (SSE_ENABLE_CONS OR SSE_ENABLE_SEARCH)
add_executable (test_renderer main.c renderer.c skull.o xmas_tree.o)
endif(SSE_ENABLE_CONS OR SSE_ENABLE_SEARCH)

target_link_libraries (test_renderer ${SDL_LIBRARY} voxtrees)
