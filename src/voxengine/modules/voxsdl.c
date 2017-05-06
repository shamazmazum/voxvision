#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <SDL2/SDL.h>
#include "../modules.h"

static void register_scancode (lua_State *L, const char *c, Uint8 scancode)
{
    struct scancodedata *data = lua_newuserdata (L, sizeof (struct scancodedata));
    luaL_getmetatable (L, "voxsdl.scancode");
    lua_setmetatable (L, -2);

    data->key = c;
    data->code = scancode;

    lua_setfield (L, -2, c);
}

static int printscancode (lua_State *L)
{
    struct scancodedata *data = luaL_checkudata (L, 1, "voxsdl.scancode");
    lua_pushfstring (L, "<scancode: '%s'>", data->key);

    return 1;
}

static const struct luaL_Reg scancode_methods [] = {
    {"__tostring", printscancode},
    {NULL, NULL}
};

static int scancodes_keypressed (lua_State *L)
{
    const Uint8 **scancodes = luaL_checkudata (L, 1, "voxsdl.scancodes");
    struct scancodedata *scancode = luaL_checkudata (L, 2, "voxsdl.scancode");
    const Uint8 *array = *scancodes;
    int res = 0;

    if (array[scancode->code]) res = 1;

    lua_pushboolean (L, res);

    return 1;
}

static const struct luaL_Reg scancodes_methods [] = {
    {"keypressed", scancodes_keypressed},
    {NULL, NULL}
};

static int get_keyboard_state (lua_State *L)
{
    const Uint8 **scancodes = lua_newuserdata (L, sizeof (void*));
    luaL_getmetatable (L, "voxsdl.scancodes");
    lua_setmetatable (L, -2);

    *scancodes = SDL_GetKeyboardState (NULL);
    return 1;
}

static const struct luaL_Reg voxsdl [] = {
    {"get_keyboard_state", get_keyboard_state},
    {NULL, NULL}
};

int luaopen_voxsdl (lua_State *L)
{
    luaL_newmetatable(L, "voxsdl.scancode");
    luaL_setfuncs (L, scancode_methods, 0);

    luaL_newmetatable(L, "voxsdl.scancodes");
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, scancodes_methods, 0);

    luaL_newlib (L, voxsdl);

    lua_newtable (L);
#include <scancodes.h>
    register_scancode (L, "leftarrow", SDL_SCANCODE_LEFT);
    register_scancode (L, "rightarrow", SDL_SCANCODE_RIGHT);
    register_scancode (L, "uparrow", SDL_SCANCODE_UP);
    register_scancode (L, "downarrow", SDL_SCANCODE_DOWN);
    lua_setfield (L, -2, "scancodes");

    return 1;
}
