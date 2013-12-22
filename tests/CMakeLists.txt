cmake_minimum_required (VERSION 2.6)

include_directories (${CUNIT_INCLUDE_DIR})
add_executable (run-tests EXCLUDE_FROM_ALL tests.c)
target_link_libraries (run-tests m voxtrees voxrnd ${CUNIT_LIBRARY})
