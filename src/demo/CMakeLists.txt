cmake_minimum_required (VERSION 2.6)

include_directories (${SDL_INCLUDE_DIR} ${INIPARSER_INCLUDE_DIR})
add_executable (demo main.c reader.c)
target_link_libraries (demo voxrnd voxtrees ${INIPARSER_LIBRARY})
