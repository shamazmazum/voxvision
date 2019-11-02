cmake_minimum_required (VERSION 2.6)
add_definitions (-DVOXTREES_SOURCE)

add_library (voxtrees SHARED
  geom.c
  geom-sse.c
  search.c
  tree.c
  datareader.c
  mtree.c)
if (WITH_DTRACE)
  include_directories (${CMAKE_CURRENT_BINARY_DIR})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/voxtrees-dtrace.d
    ${CMAKE_CURRENT_BINARY_DIR}/voxtrees-dtrace.d COPYONLY)

  add_custom_command (
    OUTPUT  voxtrees-dtrace.h
    COMMAND ${DTRACE_EXECUTABLE} -h -s voxtrees-dtrace.d -o voxtrees-dtrace.h
    DEPENDS voxtrees-dtrace.d
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  add_custom_target (voxtrees-dtrace-header ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voxtrees-dtrace.h)
  add_dependencies (voxtrees voxtrees-dtrace-header)

  set (DTRACE_SOURCES CMakeFiles/voxtrees.dir/search.c.o CMakeFiles/voxtrees.dir/tree.c.o)
  add_custom_command (TARGET voxtrees PRE_LINK
    COMMAND rm -f voxtrees-dtrace.o
    COMMAND ${DTRACE_EXECUTABLE} -G -s voxtrees-dtrace.d ${DTRACE_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set_target_properties (voxtrees PROPERTIES
    LINK_FLAGS "${CMAKE_CURRENT_BINARY_DIR}/voxtrees-dtrace.o")
endif (WITH_DTRACE)

set_target_properties (voxtrees PROPERTIES VERSION ${voxvision_VERSION}
  SOVERSION ${voxvision_VERSION_MAJOR}
  C_VISIBILITY_PRESET hidden)

target_link_libraries (voxtrees m BlocksRuntime)
install (FILES params.h tree.h search.h geom.h datareader.h mtree.h
         DESTINATION include/voxvision/voxtrees)
install (TARGETS voxtrees LIBRARY
         DESTINATION lib)
