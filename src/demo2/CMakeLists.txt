include_directories (${SDL2_INCLUDE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable (voxvision-engine main.c)
target_link_libraries (voxvision-engine voxengine voxtrees voxrnd)
install (TARGETS voxvision-engine RUNTIME
         DESTINATION bin)
