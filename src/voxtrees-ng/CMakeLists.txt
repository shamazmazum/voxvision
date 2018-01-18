cmake_minimum_required (VERSION 2.6)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/voxtrees-ng.ld.in ${CMAKE_CURRENT_BINARY_DIR}/voxtrees-ng.ld)
add_definitions (-DVOXTREES_NG_SOURCE)

add_library (voxtrees-ng SHARED datareader.c tree.c geom.c search.c)
if (WITH_DTRACE)
  include_directories (${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/voxtrees-ng-dtrace.d
    ${CMAKE_CURRENT_BINARY_DIR}/voxtrees-ng-dtrace.d COPYONLY)

  add_custom_command (
    OUTPUT  voxtrees-ng-dtrace.h
    COMMAND ${DTRACE_EXECUTABLE} -h -s voxtrees-ng-dtrace.d -o voxtrees-ng-dtrace.h
    DEPENDS voxtrees-ng-dtrace.d
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  add_custom_target (voxtrees-ng-dtrace-header ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voxtrees-ng-dtrace.h)
  add_dependencies (voxtrees-ng voxtrees-ng-dtrace-header)

  set (DTRACE_SOURCES CMakeFiles/voxtrees-ng.dir/tree.c.o CMakeFiles/voxtrees-ng.dir/search.c.o)
  add_custom_command (TARGET voxtrees-ng PRE_LINK
    COMMAND rm -f voxtrees-ng-dtrace.o
    COMMAND ${DTRACE_EXECUTABLE} -G -s voxtrees-ng-dtrace.d ${DTRACE_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set (ADDITIONAL_LINK_FLAGS "${CMAKE_CURRENT_BINARY_DIR}/voxtrees-ng-dtrace.o")
endif (WITH_DTRACE)

set_target_properties (voxtrees-ng PROPERTIES VERSION ${voxvision_VERSION}
  SOVERSION ${voxvision_VERSION_MAJOR}
  LINK_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voxtrees-ng.ld
  LINK_FLAGS "-Wl,--version-script ${CMAKE_CURRENT_BINARY_DIR}/voxtrees-ng.ld ${ADDITIONAL_LINK_FLAGS}")
target_link_libraries (voxtrees-ng BlocksRuntime m)

install (FILES types.h tree.h geom.h datareader.h search.h
  DESTINATION include/voxvision/voxtrees-ng)
install (TARGETS voxtrees-ng LIBRARY DESTINATION lib)
