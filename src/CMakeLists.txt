cmake_minimum_required (VERSION 2.6)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/voxvision.h.in ${CMAKE_CURRENT_BINARY_DIR}/voxvision.h)
include_directories (${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory (voxtrees)
add_subdirectory (voxrnd)
if (INIPARSER_FOUND)
add_subdirectory (demo)
else (INIPARSER_FOUND)
message (WARNING "Iniparser was not found. Will not build demo application.")
endif (INIPARSER_FOUND)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/voxvision.h voxtrees.h voxrnd.h
         DESTINATION include/voxvision)
