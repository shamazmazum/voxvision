#ifndef MODULES_H
#define MODULES_H

#define READ_DOT(dot,idx) do {                  \
        lua_geti (L, idx, 1);                   \
        lua_geti (L, idx, 2);                   \
        lua_geti (L, idx, 3);                   \
        dot[0] = luaL_checknumber (L, -3);      \
        dot[1] = luaL_checknumber (L, -2);      \
        dot[2] = luaL_checknumber (L, -1);      \
        lua_pop (L, 3);                         \
    } while (0)

#define WRITE_DOT(dot) do {                                 \
    lua_newtable (L);                                       \
    lua_pushnumber (L, dot[0]);                             \
    lua_pushnumber (L, dot[1]);                             \
    lua_pushnumber (L, dot[2]);                             \
    lua_seti (L, -4, 3);                                    \
    lua_seti (L, -3, 2);                                    \
    lua_seti (L, -2, 1);                                    \
    } while (0)

struct nodedata
{
    struct vox_node *node;
};

struct cameradata
{
    struct vox_camera *camera;
    struct vox_camera_interface *iface;
};

struct cddata
{
    struct vox_cd *cd;
};

struct scancodedata
{
    const char *key;
    unsigned char code;
};

#endif
