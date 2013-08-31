cmake_minimum_required (VERSION 2.6)

#add_library (voxel_octrees STATIC geom.c search.c tree.c)
add_library (voxel_octrees SHARED geom.c search.c tree.c)
target_link_libraries (voxel_octrees m)
