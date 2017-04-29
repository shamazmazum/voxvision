cmake_minimum_required (VERSION 2.6)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/voxengine.ld.in ${CMAKE_CURRENT_BINARY_DIR}/voxengine.ld)
include_directories (${LUA_INCLUDE_DIR} ${SDL2_INCLUDE_DIR}/..)
add_subdirectory (modules)

add_definitions (-fblocks -DVOXENGINE_SOURCE)
add_library (voxengine SHARED engine.c)
set_target_properties (voxengine PROPERTIES VERSION ${voxvision_VERSION}
  SOVERSION ${voxvision_VERSION_MAJOR}
  LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voxengine.ld
  LINK_FLAGS "-Wl,--version-script ${CMAKE_CURRENT_BINARY_DIR}/voxengine.ld")
target_link_libraries (voxengine voxrnd voxtrees ${LUA_LIBRARIES})

install (TARGETS voxengine LIBRARY
         DESTINATION lib)
install (FILES engine.h
         DESTINATION include/voxvision/voxengine)
