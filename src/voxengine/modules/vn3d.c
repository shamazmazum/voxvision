#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <vn3d/vn3d.h>
#include <time.h>
#include <stdlib.h>
#include "../modules.h"

static int l_noise_gen_gc (lua_State *L)
{
    struct vn_generator **data = luaL_checkudata (L, 1, NOISE_GEN);
    vn_destroy_generator (*data);
    *data = NULL;

    return 0;
}

static int l_noise_gen_tostring (lua_State *L)
{
    lua_pushstring (L, "<noise generator>");
    return 1;
}

static int l_getnoise (lua_State *L)
{
    struct vn_generator **data = luaL_checkudata (L, 1, NOISE_GEN);
    struct vn_generator *gen = *data;
    size_t len = lua_rawlen (L, 2);
    if (len != 3) {
        lua_pushnil (L);
        lua_pushstring (L, "argument table is not three elements long");
        return 2;
    }
    unsigned int x,y,z,val;
    READ_DOT_3 (2, x, y, z);
    val = vn_noise_3d (gen, x, y, z);
    lua_pushnumber (L, (float)val/0xffffffff);
    return 1;
}

static const struct luaL_Reg noisegen_methods[] = {
    {"__gc", l_noise_gen_gc},
    {"__tostring", l_noise_gen_tostring},
    {"getnoise", l_getnoise},
    {NULL, NULL}
};

static int l_new_noise_gen (lua_State *L)
{
    unsigned int octaves = luaL_checkinteger (L, 1);
    unsigned int width = luaL_checkinteger (L, 2);
    unsigned int height = luaL_checkinteger (L, 3);
    unsigned int depth = luaL_checkinteger (L, 4);
    struct vn_generator *generator = vn_make_generator (octaves, width, height, depth);
    int res;

    // FIXME: Report an error
    if (generator == NULL) {
        lua_pushnil (L);
        lua_pushstring (L, vn_get_error_msg ());
        res = 2;
    } else {
        struct vn_generator **data = lua_newuserdata (L, sizeof (struct vn_generator*));
        luaL_getmetatable (L, NOISE_GEN);
        lua_setmetatable (L, -2);
        *data = generator;
        res = 1;
    }

    return res;
}

static int l_randomize (lua_State *L)
{
    srand (time (NULL));
    return 0;
}

static const struct luaL_Reg vn3d[] = {
    {"noise_gen", l_new_noise_gen},
    {"randomize", l_randomize},
    {NULL, NULL}
};

int luaopen_vn3d (lua_State *L)
{
    luaL_newmetatable(L, NOISE_GEN);
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, noisegen_methods, 0);

    luaL_newlib (L, vn3d);
    return 1;
}
