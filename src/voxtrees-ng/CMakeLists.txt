cmake_minimum_required (VERSION 2.6)

add_definitions (-DVOXTREES_NG_SOURCE)

add_library (voxtrees-ng SHARED datareader.c tree.c geom.c)
target_link_libraries (voxtrees-ng BlocksRuntime m)
