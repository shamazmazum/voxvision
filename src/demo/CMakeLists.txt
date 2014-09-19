cmake_minimum_required (VERSION 2.6)

include_directories (${SDL_INCLUDE_DIR} ${INIPARSER_INCLUDE_DIR} ..)
add_executable (voxvision-demo main.c reader.c)
target_link_libraries (voxvision-demo voxrnd voxtrees ${INIPARSER_LIBRARY})
install (TARGETS voxvision-demo RUNTIME
         DESTINATION bin)
