find_path(SDL2_INCLUDE_DIR SDL.h
          PATH_SUFFIXES SDL2)
list(APPEND SDL_INCLUDE_DIR /usr/local/include)
mark_as_advanced(SDL2_INCLUDE_DIR)

find_library(SDL2_LIBRARY NAMES
sdl2
SDL2
libsdl2
libSDL2
sdl2lib
)
mark_as_advanced(SDL2_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2 DEFAULT_MSG SDL2_LIBRARY SDL2_INCLUDE_DIR)
