cmake_minimum_required (VERSION 2.6)

include_directories (${SDL_INCLUDE_DIR})

if (SSE_INTRIN)
add_executable (demo main.c skull_sse.o xmas_tree_sse.o) 
else (SSE_INTRIN)
add_executable (demo main.c skull.o xmas_tree.o)
endif(SSE_INTRIN)

target_link_libraries (demo voxrnd voxtrees)
