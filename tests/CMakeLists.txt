include_directories (${CUNIT_INCLUDE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR}/../src
                     ${CMAKE_CURRENT_SOURCE_DIR}/../src)

# Allow tests to see internals
add_definitions (-DVOXTREES_SOURCE -DVOXRND_SOURCE -fblocks)
add_executable (run-tests EXCLUDE_FROM_ALL tests.c)
target_link_libraries (run-tests m voxtrees voxrnd ${CUNIT_LIBRARY})

add_custom_target (check
  env "VOXVISION_MODULES=${CMAKE_BINARY_DIR}/src/voxrnd" ./run-tests
  COMMENT "Run tests"
  DEPENDS run-tests doom-camera-module simple-camera-module
  )
