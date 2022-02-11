configure_file (${CMAKE_CURRENT_SOURCE_DIR}/loadenvironment.lua.in
  ${CMAKE_CURRENT_BINARY_DIR}/loadenvironment.lua)
include_directories (${LUA_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR})

if (GCD_FOUND)
include_directories (${GCD_INCLUDE_DIR})
add_definitions (-DUSE_GCD)
endif (GCD_FOUND)
add_subdirectory (modules)

add_definitions (-DVOXENGINE_SOURCE)
add_library (voxengine SHARED engine.c)
set_target_properties (voxengine PROPERTIES VERSION ${voxvision_VERSION}
  SOVERSION ${voxvision_VERSION_MAJOR}
  C_VISIBILITY_PRESET hidden)
target_link_libraries (voxengine ${LUA_LIBRARIES} BlocksRuntime)
if (GCD_FOUND)
target_link_libraries (voxengine ${GCD_LIBRARY})
endif (GCD_FOUND)

install (TARGETS voxengine LIBRARY
  DESTINATION lib)
install (FILES engine.h
  DESTINATION include/voxvision/voxengine)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/loadenvironment.lua
  DESTINATION share/voxvision/modules)
