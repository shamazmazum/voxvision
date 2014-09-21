cmake_minimum_required (VERSION 2.6)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/voxvision.h.in ${CMAKE_CURRENT_SOURCE_DIR}/voxvision.h)

add_subdirectory (voxtrees)
add_subdirectory (voxrnd)
add_subdirectory (demo)

install (FILES params_var.h voxtrees.h voxrnd.h
         DESTINATION include/voxvision)
