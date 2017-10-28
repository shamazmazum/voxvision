#ifndef MODULES_H
#define MODULES_H

#define READ_DOT(dot,idx) do {                    \
        lua_geti (L, idx, 1);                     \
        lua_geti (L, idx, 2);                     \
        lua_geti (L, idx, 3);                     \
        float t1 = luaL_checknumber (L, -3);      \
        float t2 = luaL_checknumber (L, -2);      \
        float t3 = luaL_checknumber (L, -1);      \
        vox_dot_set (dot, t1, t2, t3);            \
        lua_pop (L, 3);                           \
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

struct cameradata
{
    struct vox_camera *camera;
    struct vox_camera_interface *iface;
};

#endif
