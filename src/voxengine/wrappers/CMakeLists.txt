cmake_minimum_required (VERSION 2.6)

include_directories (${LUA_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${SDL2_INCLUDE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR}/..)
add_definitions (-fblocks)

add_library (voxtrees-wrapper SHARED voxtrees.c)
set_target_properties (voxtrees-wrapper PROPERTIES OUTPUT_NAME voxtrees PREFIX "")

add_library (voxrnd-wrapper SHARED voxrnd.c)
set_target_properties (voxrnd-wrapper PROPERTIES OUTPUT_NAME voxrnd PREFIX "")

install (TARGETS voxtrees-wrapper voxrnd-wrapper LIBRARY
         DESTINATION share/voxvision/modules)
