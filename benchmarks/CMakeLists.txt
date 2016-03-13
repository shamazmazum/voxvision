cmake_minimum_required (VERSION 2.6)

add_library (gettime STATIC EXCLUDE_FROM_ALL gettime.c)

add_subdirectory (voxtrees)
add_subdirectory (voxrnd)

add_custom_target (benchmarks
                   DEPENDS benchmarks-voxtrees benchmarks-voxrnd)
add_custom_target (run-benchmarks
                   DEPENDS run-benchmarks-voxtrees run-benchmarks-voxrnd)
