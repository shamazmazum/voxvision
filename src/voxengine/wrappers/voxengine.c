#include <sys/param.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <modules.h>
#include "../engine.h"

static int find_data_file (lua_State *L)
{
    char fullpath[MAXPATHLEN];
    const char *filename = luaL_checkstring (L, 1);
    int res;

    if (vox_find_data_file (filename, fullpath))
    {
        lua_pushstring (L, fullpath);
        res = 1;
    }
    else
    {
        lua_pushnil (L);
        lua_pushstring (L, "No such file");
        res = 2;
    }

    return res;
}

static const struct luaL_Reg voxengine [] = {
    {"find_data_file", find_data_file},
    {NULL, NULL}
};

int luaopen_voxengine (lua_State *L)
{
    luaL_newlib (L, voxengine);
    return 1;
}
