cmake_minimum_required (VERSION 2.6)

add_executable (test_renderer main.c renderer.c skull.c xmas_tree.c)
target_link_libraries (test_renderer ${SDL_LIBRARY} voxel_octrees)
