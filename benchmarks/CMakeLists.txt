cmake_minimum_required (VERSION 2.6)

add_library (gettime STATIC EXCLUDE_FROM_ALL gettime.c)

include_directories (../src)
file (GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} b-*.c)
foreach (SOURCE ${SOURCES})
    get_filename_component (TARGET_NAME ${SOURCE} NAME_WE)
    add_executable (${TARGET_NAME} EXCLUDE_FROM_ALL ${SOURCE})
    target_link_libraries (${TARGET_NAME} voxtrees gettime)
    set (TARGET_NAMES ${TARGET_NAMES} ${TARGET_NAME})
endforeach (SOURCE)

add_custom_target (benchmarks DEPENDS ${TARGET_NAMES})
