cmake_minimum_required (VERSION 2.6)

set (voxengine_module_path ${CMAKE_INSTALL_PREFIX}/share/voxvision/modules)
set (voxengine_data_path ${CMAKE_INSTALL_PREFIX}/share/voxvision/data)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/voxvision.h.in ${CMAKE_CURRENT_BINARY_DIR}/voxvision.h)
include_directories (${CMAKE_CURRENT_BINARY_DIR})

add_definitions (-fblocks)
add_subdirectory (voxtrees)
add_subdirectory (voxrnd)
if (INIPARSER_FOUND)
add_subdirectory (demo)
else (INIPARSER_FOUND)
message (WARNING "Iniparser was not found. Will not build demo application.")
endif (INIPARSER_FOUND)

if (LUA_FOUND)
message (STATUS "Lua version OK.")
add_subdirectory (voxengine)
add_subdirectory (demo2)
endif (LUA_FOUND)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/voxvision.h voxtrees.h voxrnd.h voxengine.h
         DESTINATION include/voxvision)
