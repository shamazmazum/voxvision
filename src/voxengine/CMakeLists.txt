cmake_minimum_required (VERSION 2.6)

add_custom_command (OUTPUT safe_environment.h
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/genenvironment.py > ${CMAKE_CURRENT_BINARY_DIR}/safe_environment.h
  DEPENDS genenvironment.py)
add_custom_target (safe_environment ALL
  DEPENDS safe_environment.h)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/voxengine.ld.in ${CMAKE_CURRENT_BINARY_DIR}/voxengine.ld)
include_directories (${LUA_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIR}/..
  ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory (modules)

add_definitions (-fblocks -DVOXENGINE_SOURCE)
add_library (voxengine SHARED engine.c safe_environment)
set_target_properties (voxengine PROPERTIES VERSION ${voxvision_VERSION}
  SOVERSION ${voxvision_VERSION_MAJOR}
  LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voxengine.ld
  LINK_FLAGS "-Wl,--version-script ${CMAKE_CURRENT_BINARY_DIR}/voxengine.ld")
target_link_libraries (voxengine ${LUA_LIBRARIES} BlocksRuntime)

install (TARGETS voxengine LIBRARY
         DESTINATION lib)
install (FILES engine.h
         DESTINATION include/voxvision/voxengine)