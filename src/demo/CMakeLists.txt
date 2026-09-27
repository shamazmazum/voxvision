include_directories (${SDL2_INCLUDE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/..)
if (GCD_FOUND)
include_directories (${GCD_INCLUDE_DIRS})
add_definitions (-DUSE_GCD)
endif (GCD_FOUND)
add_definitions (-fblocks)

add_executable (voxvision-demo main.c config.c)
target_link_libraries (voxvision-demo voxrnd voxtrees iniparser-shared)
install (TARGETS voxvision-demo RUNTIME
         DESTINATION bin)
