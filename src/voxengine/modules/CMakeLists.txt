cmake_minimum_required (VERSION 2.6)

include_directories (${LUA_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${CMAKE_CURRENT_BINARY_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR})

add_library (voxtrees-wrapper SHARED voxtrees.c)
set_target_properties (voxtrees-wrapper PROPERTIES OUTPUT_NAME voxtrees PREFIX "")

add_library (voxrnd-wrapper SHARED voxrnd.c)
set_target_properties (voxrnd-wrapper PROPERTIES OUTPUT_NAME voxrnd PREFIX "")

if (VN3D_FOUND)
  add_library (vn3d-wrapper SHARED vn3d.c)
  set_target_properties (vn3d-wrapper PROPERTIES OUTPUT_NAME vn3d PREFIX "")
  target_include_directories (vn3d-wrapper PRIVATE ${VN3D_INCLUDE_DIR})
  target_link_libraries (vn3d-wrapper ${VN3D_LIBRARY})
endif (VN3D_FOUND)

add_subdirectory (luasdl2)

install (TARGETS voxtrees-wrapper voxrnd-wrapper LIBRARY
  DESTINATION share/voxvision/modules)

install (FILES voxutils.lua
  DESTINATION share/voxvision/modules)

if (VN3D_FOUND)
  install (TARGETS vn3d-wrapper LIBRARY
    DESTINATION share/voxvision/modules)
endif (VN3D_FOUND)
