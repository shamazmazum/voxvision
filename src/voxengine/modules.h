#ifndef MODULES_H
#define MODULES_H
#ifdef USE_GCD
#include <dispatch/dispatch.h>
#else
#include "../gcd-stubs.c"
#endif

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

#define READ_DOT_3(idx, x, y, z) do {             \
        lua_geti (L, idx, 1);                     \
        lua_geti (L, idx, 2);                     \
        lua_geti (L, idx, 3);                     \
        x = luaL_checknumber (L, -3);             \
        y = luaL_checknumber (L, -2);             \
        z = luaL_checknumber (L, -1);             \
        lua_pop (L, 3);                           \
    } while (0)

#define WRITE_DOT(dot) do {                                 \
        lua_newtable (L);                                   \
        lua_pushnumber (L, dot[0]);                         \
        lua_pushnumber (L, dot[1]);                         \
        lua_pushnumber (L, dot[2]);                         \
        lua_seti (L, -4, 3);                                \
        lua_seti (L, -3, 2);                                \
        lua_seti (L, -2, 1);                                \
    } while (0)

#define WRITE_DOT_3(x,y,z) do {                             \
        lua_newtable (L);                                   \
        lua_pushnumber (L, x);                              \
        lua_pushnumber (L, y);                              \
        lua_pushnumber (L, z);                              \
        lua_seti (L, -4, 3);                                \
        lua_seti (L, -3, 2);                                \
        lua_seti (L, -2, 1);                                \
    } while (0)

struct cameradata
{
    struct vox_camera *camera;
    struct vox_camera_interface *iface;
};

struct scene_proxydata
{
    struct vox_node *tree;
    struct vox_rnd_ctx *context;
    dispatch_group_t scene_group;
    dispatch_queue_t scene_sync_queue;
};

#define TREE_META "voxtrees.vox_node"
#define DOTMAP_META "voxtrees.dotmap"
#define CAMERA_META "voxrnd.camera"
#define CD_META "voxrnd.cd"
#define SCENE_PROXY_META "voxrnd.scene_proxy"

#endif
