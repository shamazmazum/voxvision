cmake_minimum_required (VERSION 2.6)

include_directories (../../src ..)
file (GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c)
foreach (SOURCE ${SOURCES})
    get_filename_component (TARGET_NAME ${SOURCE} NAME_WE)
    add_executable (${TARGET_NAME} EXCLUDE_FROM_ALL ${SOURCE})
    target_link_libraries (${TARGET_NAME} voxtrees voxrnd gettime)
    set (TARGET_NAMES ${TARGET_NAMES} ${TARGET_NAME})
    add_custom_target (run-${TARGET_NAME} ./${TARGET_NAME}
                       DEPENDS ${TARGET_NAME})
    set (RUN_NAMES ${RUN_NAMES} run-${TARGET_NAME})
endforeach (SOURCE)

add_custom_target (benchmarks-voxrnd DEPENDS ${TARGET_NAMES})
add_custom_target (run-benchmarks-voxrnd DEPENDS ${RUN_NAMES})
