cmake_minimum_required (VERSION 2.6)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/params_var.h.in ${CMAKE_CURRENT_SOURCE_DIR}/params_var.h)

add_subdirectory (voxtrees)
#add_subdirectory (simple-renderer)
add_subdirectory (voxrnd)
add_subdirectory (demo)
