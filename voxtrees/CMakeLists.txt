cmake_minimum_required (VERSION 2.6)

#add_library (voxtrees SHARED geom.c search.c tree.c list.c)
add_library (voxtrees SHARED geom.c search.c tree.c)

#target_link_libraries (voxtrees m ${BOEHM_GC_LIBRARIES})
target_link_libraries (voxtrees m)
