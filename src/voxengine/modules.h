#ifndef MODULES_H
#define MODULES_H

#ifdef SSE_INTRIN
#define store_dot(dot, x, y, z) _mm_store_ps (dot, _mm_set_ps (0, z, y, x))
#else
#define store_dot(dot, x, y, z) do {            \
        dot[0] = x; dot[1] = y; dot[2] = z;     \
    } while (0)
#endif

#define READ_DOT(dot,idx) do {                    \
        lua_geti (L, idx, 1);                     \
        lua_geti (L, idx, 2);                     \
        lua_geti (L, idx, 3);                     \
        float t1 = luaL_checknumber (L, -3);      \
        float t2 = luaL_checknumber (L, -2);      \
        float t3 = luaL_checknumber (L, -1);      \
        store_dot (dot, t1, t2, t3);              \
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
