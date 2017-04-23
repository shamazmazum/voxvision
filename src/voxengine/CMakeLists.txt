cmake_minimum_required (VERSION 2.6)

set (voxengine_module_path ${CMAKE_INSTALL_PREFIX}/share/voxvision/modules)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/voxengine.ld.in ${CMAKE_CURRENT_BINARY_DIR}/voxengine.ld)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/modules.h.in ${CMAKE_CURRENT_BINARY_DIR}/modules.h)
include_directories (${LUA_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${SDL2_INCLUDE_DIR}/..)
include_directories (${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory (wrappers)

add_definitions (-fblocks)
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
