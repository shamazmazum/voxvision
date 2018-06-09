cmake_minimum_required (VERSION 2.6)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/voxrnd.ld.in ${CMAKE_CURRENT_BINARY_DIR}/voxrnd.ld)
include_directories (${SDL2_INCLUDE_DIR}/..)
add_definitions (-DVOXRND_SOURCE)

if (GCD_FOUND)
include_directories (${GCD_INCLUDE_DIR})
add_definitions (-DUSE_GCD)
endif (GCD_FOUND)

if (WITH_LIGHT_MANAGER)
  add_definitions (-DLIGHT_MANAGER)
endif (WITH_LIGHT_MANAGER)

set (VOXRND_SOURCES
  object.c
  modules.c
  lights.c
  camera.c
  renderer.c
  vect-ops.c
  vect-ops-sse.c
  fps-control.c
  cd.c
  copy-helper.c)

add_library (voxrnd SHARED ${VOXRND_SOURCES})
if (WITH_DTRACE)
  include_directories (${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/voxrnd-dtrace.d
    ${CMAKE_CURRENT_BINARY_DIR}/voxrnd-dtrace.d COPYONLY)

  add_custom_command (
    OUTPUT  voxrnd-dtrace.h
    COMMAND ${DTRACE_EXECUTABLE} -h -s voxrnd-dtrace.d -o voxrnd-dtrace.h
    DEPENDS voxrnd-dtrace.d
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  add_custom_target (voxrnd-dtrace-header ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voxrnd-dtrace.h)
  add_dependencies (voxrnd voxrnd-dtrace-header)

  add_custom_command (TARGET voxrnd PRE_LINK
    COMMAND rm -f voxrnd-dtrace.o
    COMMAND ${DTRACE_EXECUTABLE} -G -s voxrnd-dtrace.d CMakeFiles/voxrnd.dir/renderer.c.o
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set (ADDITIONAL_LINK_FLAGS "${CMAKE_CURRENT_BINARY_DIR}/voxrnd-dtrace.o")
endif (WITH_DTRACE)
set_target_properties (voxrnd PROPERTIES VERSION ${voxvision_VERSION}
                                         SOVERSION ${voxvision_VERSION_MAJOR}
                                         LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voxrnd.ld
                                         LINK_FLAGS "-Wl,--version-script ${CMAKE_CURRENT_BINARY_DIR}/voxrnd.ld ${ADDITIONAL_LINK_FLAGS}")

target_link_libraries (voxrnd ${SDL2_LIBRARY} m BlocksRuntime)
if (GCD_FOUND)
target_link_libraries (voxrnd ${GCD_LIBRARY})
endif (GCD_FOUND)

add_library (simple-camera-module SHARED simple-camera.c)
set_target_properties (simple-camera-module PROPERTIES OUTPUT_NAME simple-camera PREFIX "")

add_library (doom-camera-module SHARED doom-camera.c)
set_target_properties (doom-camera-module PROPERTIES OUTPUT_NAME doom-camera PREFIX "")

install (FILES camera.h renderer.h fps-control.h cd.h lights.h modules.h object.h
         DESTINATION include/voxvision/voxrnd)
install (TARGETS voxrnd LIBRARY
         DESTINATION lib)
install (TARGETS simple-camera-module doom-camera-module LIBRARY
         DESTINATION share/voxvision/modules)
