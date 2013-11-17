cmake_minimum_required (VERSION 2.6)

include_directories (${CUNIT_INCLUDE_DIR})
add_executable (run-tests tests.c)
target_link_libraries (run-tests m voxtrees vect-ops ${CUNIT_LIBRARY})
