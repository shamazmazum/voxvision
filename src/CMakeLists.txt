cmake_minimum_required (VERSION 2.6)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/voxvision.h.in ${CMAKE_CURRENT_BINARY_DIR}/voxvision.h)
include_directories (${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory (voxtrees)
add_subdirectory (voxrnd)
add_subdirectory (demo)

install (FILES voxvision.h voxtrees.h voxrnd.h
         DESTINATION include/voxvision)
