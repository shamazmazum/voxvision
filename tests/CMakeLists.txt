cmake_minimum_required (VERSION 2.6)

include_directories (${CUNIT_INCLUDE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}/../src
                     ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_definitions (-fblocks)                   
add_executable (run-tests EXCLUDE_FROM_ALL tests.c)
# Allow tests to see internals
target_compile_definitions  (run-tests PRIVATE -DVOXTREES_SOURCE -DVOXRND_SOURCE)

add_executable (run-new-tests EXCLUDE_FROM_ALL new-tests.c)
target_compile_definitions  (run-new-tests PRIVATE -DDATAPATH="${CMAKE_SOURCE_DIR}/data"
  -DVOXTREES_NG_SOURCE)

target_link_libraries (run-tests m voxtrees voxrnd ${CUNIT_LIBRARY})
target_link_libraries (run-new-tests voxtrees-ng  ${CUNIT_LIBRARY})

add_custom_target (check
  env "VOXVISION_MODULES=${CMAKE_BINARY_DIR}/src/voxrnd" ./run-tests && ./run-new-tests
  COMMENT "Run tests"
  DEPENDS run-tests run-new-tests doom-camera-module simple-camera-module
  )
