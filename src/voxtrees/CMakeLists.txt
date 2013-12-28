cmake_minimum_required (VERSION 2.6)

#add_library (voxtrees SHARED geom.c search.c tree.c list.c)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/voxtrees.ld.in ${CMAKE_CURRENT_BINARY_DIR}/voxtrees.ld)
add_library (voxtrees SHARED geom.c search.c tree.c)

set_target_properties (voxtrees PROPERTIES VERSION ${voxvision_VERSION}
                                           SOVERSION ${voxvision_VERSION_MAJOR}
                                           LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voxtrees.ld
                                           LINK_FLAGS "-Wl,--version-script ${CMAKE_CURRENT_BINARY_DIR}/voxtrees.ld")

#target_link_libraries (voxtrees m ${BOEHM_GC_LIBRARIES})
target_link_libraries (voxtrees m)
