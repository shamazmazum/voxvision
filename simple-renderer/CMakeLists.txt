cmake_minimum_required (VERSION 2.6)

add_executable (test_renderer main.c renderer.c skull.o xmas_tree.o)
target_link_libraries (test_renderer ${SDL_LIBRARY} voxtrees)
