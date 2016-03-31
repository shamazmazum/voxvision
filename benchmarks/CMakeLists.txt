cmake_minimum_required (VERSION 2.6)

add_library (gettime STATIC EXCLUDE_FROM_ALL gettime.c)

include_directories (${CMAKE_CURRENT_BINARY_DIR}/../src
                     ${CMAKE_CURRENT_SOURCE_DIR}/../src
                     ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory (voxtrees)
add_subdirectory (voxrnd)

add_custom_target (benchmarks
                   DEPENDS benchmarks-voxtrees benchmarks-voxrnd)
add_custom_target (run-benchmarks
                   DEPENDS run-benchmarks-voxtrees run-benchmarks-voxrnd)
