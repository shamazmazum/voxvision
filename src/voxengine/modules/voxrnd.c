#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <voxrnd.h>
#include <voxtrees.h>
#include <stdlib.h>
#include <string.h>
#include "../modules.h"

static int get_position (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, CAMERA_META);
    vox_dot position;
    camera->iface->get_position (camera->camera, position);
    WRITE_DOT (position);

    return 1;
}

static int set_position (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, CAMERA_META);
    vox_dot position;
    READ_DOT (position, 2);
    camera->iface->set_position (camera->camera, position);

    return 0;
}

static int get_fov (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, CAMERA_META);
    lua_pushnumber (L, camera->iface->get_fov (camera->camera));

    return 1;
}

static int set_fov (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, CAMERA_META);
    float fov = luaL_checknumber (L, 2);
    camera->iface->set_fov (camera->camera, fov);

    return 0;
}

static int set_rot_angles (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, CAMERA_META);
    vox_dot angles;
    READ_DOT (angles, 2);
    camera->iface->set_rot_angles (camera->camera, angles);

    return 0;
}

static int rotate_camera (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, CAMERA_META);
    vox_dot delta;
    READ_DOT (delta, 2);
    camera->iface->rotate_camera (camera->camera, delta);

    return 0;
}

static int move_camera (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, CAMERA_META);
    vox_dot delta;
    READ_DOT (delta, 2);
    camera->iface->move_camera (camera->camera, delta);

    return 0;
}

static int look_at (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, CAMERA_META);
    vox_dot coord;
    READ_DOT (coord, 2);
    camera->iface->look_at (camera->camera, coord);

    return 0;
}

static int l_camera_screen2world (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, CAMERA_META);
    int x = luaL_checkinteger (L, 2);
    int y = luaL_checkinteger (L, 3);
    vox_dot res;

    camera->iface->screen2world (camera->camera, res, x, y);
    WRITE_DOT (res);

    return 1;
}

static int new_camera (lua_State *L)
{
    const char *name = luaL_checkstring (L, 1);
    struct vox_camera_interface *iface = vox_camera_methods (name);
    struct cameradata *data;
    int nret = 0;

    if (iface != NULL) {
        data = lua_newuserdata (L, sizeof (struct cameradata));
        data->camera = iface->construct_camera (NULL);
        data->iface = data->camera->iface;
        luaL_getmetatable (L, CAMERA_META);
        lua_setmetatable (L, -2);
        nret = 1;
    } else {
        lua_pushnil (L);
        lua_pushstring (L, "Cannot load camera");
        nret = 2;
    }

    return nret;
}

static int destroycamera (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, CAMERA_META);
    camera->iface->destroy_camera (camera->camera);

    return 0;
}

static int printcamera (lua_State *L)
{
    lua_pushfstring (L, "<camera %p>", lua_topointer (L, 1));
    return 1;
}

static const struct luaL_Reg camera_methods [] = {
    {"__gc", destroycamera},
    {"__tostring", printcamera},
    {"get_position", get_position},
    {"set_position", set_position},
    {"get_fov", get_fov},
    {"set_fov", set_fov},
    {"set_rot_angles", set_rot_angles},
    {"rotate_camera", rotate_camera},
    {"move_camera", move_camera},
    {"look_at", look_at},
    {"screen2world", l_camera_screen2world},
    {NULL, NULL}
};

static int new_cd (lua_State *L)
{
    struct vox_cd **data = lua_newuserdata (L, sizeof (struct vox_cd**));
    *data = vox_make_cd ();
    luaL_getmetatable (L, CD_META);
    lua_setmetatable (L, -2);
    return 1;
}

static int printcd (lua_State *L)
{
    lua_pushfstring (L, "<collision detector %p>", lua_topointer (L, 1));
    return 1;
}

static int destroycd (lua_State *L)
{
    struct vox_cd **cd = luaL_checkudata (L, 1, CD_META);
    free (*cd);

    return 0;
}

static int cd_attach_camera (lua_State *L)
{
    struct vox_cd **cd = luaL_checkudata (L, 1, CD_META);
    struct cameradata *camera = luaL_checkudata (L, 2, CAMERA_META);
    float radius = luaL_checknumber (L, 3);

    vox_cd_attach_camera (*cd, camera->camera, radius);

    return 0;
}

static int cd_attach_context (lua_State *L)
{
    struct vox_cd **cd = luaL_checkudata (L, 1, CD_META);
    struct context_data *ctx = luaL_checkudata (L, 2, CONTEXT_META);

    vox_cd_attach_context (*cd, ctx->context);
    lua_pushvalue (L, 1);
    lua_setfield (L, 2, "cd");

    return 0;
}

static int cd_gravity (lua_State *L)
{
    struct vox_cd **cd = luaL_checkudata (L, 1, CD_META);
    vox_dot gravity;
    READ_DOT (gravity, 2);

    vox_cd_gravity (*cd, gravity);

    return 0;
}

static int cd_collide (lua_State *L)
{
    struct vox_cd **cd = luaL_checkudata (L, 1, CD_META);

    vox_cd_collide (*cd);
    return 0;
}

static const struct luaL_Reg cd_methods [] = {
    {"__gc", destroycd},
    {"__tostring", printcd},
    {"attach_camera", cd_attach_camera},
    {"attach_context", cd_attach_context},
    {"collide", cd_collide},
    {"gravity", cd_gravity},
    {NULL, NULL}
};

static int l_scene_proxy_tostring (lua_State *L)
{
    lua_pushfstring (L, "<async scene proxy %p>", lua_topointer (L, 1));
    return 1;
}

static int l_scene_proxy_insert (lua_State *L)
{
    struct vox_node **ndata;
    struct scene_proxydata *data = luaL_checkudata (L, 1, SCENE_PROXY_META);
    float x,y,z;
    READ_DOT_3 (2, x, y, z);

    lua_getfield (L, 1, "__tree");
    ndata = luaL_checkudata (L, -1, TREE_META);

    /*
     * Enqueue insertion to a synchronous queue associated with the tree. The
     * insertion will be performed when there are no other jobs in the tree
     * group. For example if there is tree rebuilding or rendering in progress,
     * wait for its completion first, and then insert a voxel.
     */
    dispatch_group_notify (data->scene_group, data->scene_sync_queue, ^{
            vox_dot dot;
            vox_dot_set (dot, x, y, z);
            vox_insert_voxel (&(data->tree), dot);
            *ndata = data->tree;
            vox_context_set_scene (data->context, data->tree);
        });

    return 0;
}

static int l_scene_proxy_delete (lua_State *L)
{
    struct vox_node **ndata;
    struct scene_proxydata *data = luaL_checkudata (L, 1, SCENE_PROXY_META);
    float x,y,z;
    READ_DOT_3 (2, x, y, z);

    lua_getfield (L, 1, "__tree");
    ndata = luaL_checkudata (L, -1, TREE_META);

    /* See the comment for l_scene_proxy_insert */
    dispatch_group_notify (data->scene_group, data->scene_sync_queue, ^{
            vox_dot dot;
            vox_dot_set (dot, x, y, z);
            vox_delete_voxel (&(data->tree), dot);
            *ndata = data->tree;
            vox_context_set_scene (data->context, data->tree);
        });

    return 0;
}

static int l_scene_proxy_rebuild (lua_State *L)
{
    struct vox_node **ndata;
    struct scene_proxydata *data = luaL_checkudata (L, 1, SCENE_PROXY_META);

    lua_getfield (L, 1, "__tree");
    ndata = luaL_checkudata (L, -1, TREE_META);

    /*
     * Asynchronously rebuild the tree and then enqueue the new tree for
     * replacement.
     */
    dispatch_group_async (data->scene_group,
                          dispatch_get_global_queue
                          (DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
                              struct vox_node *new_tree = vox_rebuild_tree (data->tree);
                              dispatch_sync (data->scene_sync_queue, ^{
                                      vox_destroy_tree (data->tree);
                                      data->tree = new_tree;
                                      /* Update lua reference */
                                      *ndata = new_tree;
                                      vox_context_set_scene (data->context, new_tree);
                                  });
                          });
    return 0;
}

static int l_scene_proxy_ray_intersection (lua_State *L)
{
    struct scene_proxydata *data = luaL_checkudata (L, 1, SCENE_PROXY_META);
    __block const struct vox_node *leaf;
    float o1, o2, o3;
    float d1, d2, d3;
    READ_DOT_3 (2, o1, o2, o3);
    READ_DOT_3 (3, d1, d2, d3);

    __block float r1, r2, r3;
    /*
     * To operate on a consistent state, enqueue search operation in sync
     * queue.
     */
    dispatch_sync (data->scene_sync_queue, ^{
            vox_dot origin, dir, res;
            vox_dot_set (origin, o1, o2, o3);
            vox_dot_set (dir, d1, d2, d3);
            leaf = vox_ray_tree_intersection (data->tree, origin, dir, res);
            r1 = res[0]; r2 = res[1]; r3 = res[2];
        });

    if (leaf != NULL) WRITE_DOT_3 (r1, r2, r3);
    else lua_pushnil (L);
    return 1;
}

/*
 * Unfortunately, this is not actually a proxy, as it does not translate all
 * method calls to underlying tree. It just overwrites some methods of the tree
 * with a thread-safe equivalents, prohibiting the others.
 */
static const struct luaL_Reg scene_proxy_methods [] = {
    {"__tostring", l_scene_proxy_tostring},
    {"rebuild", l_scene_proxy_rebuild},
    {"insert", l_scene_proxy_insert},
    {"delete", l_scene_proxy_delete},
    {"ray_intersection", l_scene_proxy_ray_intersection},
    {NULL, NULL}
};

static int l_context_tostring (lua_State *L)
{
    lua_pushstring (L, "<renderer context>");
    return 1;
}

static int l_context_destroy (lua_State *L)
{
    struct context_data *data = luaL_checkudata (L, 1, CONTEXT_META);

    dispatch_release (data->rendering_queue);
    dispatch_release (data->rendering_group);
    vox_destroy_context (data->context);
    return 0;
}

static int l_context_geometry (lua_State *L)
{
    struct context_data *data = luaL_checkudata (L, 1, CONTEXT_META);
    struct vox_rnd_ctx *ctx = data->context;

    lua_pushinteger (L, ctx->surface->w);
    lua_pushinteger (L, ctx->surface->h);
    return 2;
}

static int l_context_newindex (lua_State *L)
{
    struct context_data *data = luaL_checkudata (L, 1, CONTEXT_META);
    struct vox_rnd_ctx *ctx = data->context;
    const char *field = luaL_checkstring (L, 2);

    luaL_getmetatable (L, CONTEXT_META);
    if (strcmp (field, "tree") == 0) {
        /* FIXME: I need to use GCD sync to properly change a tree here */
        struct vox_node **ndata = luaL_checkudata (L, 3, TREE_META);
        struct vox_node *scene = *ndata;

        /* Provide access to the tree via asyncronous proxy */
        struct scene_proxydata *pdata = lua_newuserdata (L, sizeof (struct scene_proxydata));
        pdata->tree = scene;
        pdata->context = ctx;
        pdata->scene_group = data->rendering_group;
        pdata->scene_sync_queue = data->rendering_queue;
        /*
         * KLUDGE:
         * We must wait here for all operations on the tree to complete.
         * Unfortunately, we cannot add the following actions in queue (e.g. via
         * dispatch_group_notify) because it will mess with lua state.
         */
        dispatch_group_wait (data->rendering_group, DISPATCH_TIME_FOREVER);
        vox_context_set_scene (ctx, pdata->tree);

        luaL_getmetatable (L, SCENE_PROXY_META);
        lua_pushvalue (L, 3);
        lua_setfield (L, -2, "__tree"); // To prevent gc'ing
        lua_setmetatable (L, -2);
        lua_setfield (L, -2, "tree");
    } else if (strcmp (field, "camera") == 0) {
        struct cameradata *cdata = luaL_checkudata (L, 3, CAMERA_META);
        vox_context_set_camera (ctx, cdata->camera);

        lua_pushvalue (L, 3);
        lua_setfield (L, -2, "camera");
    } else {
        // Allow user to store anything world-related in the world table.
        lua_pushvalue (L, 3);
        lua_setfield (L, -2, field);
    }

    return 0;
}

static int l_context_rendering_mode (lua_State *L)
{
    struct context_data *data = luaL_checkudata (L, 1, CONTEXT_META);
    struct vox_rnd_ctx *ctx = data->context;
    unsigned int mode = luaL_checkinteger (L, 2);
    int res = vox_context_set_quality (ctx, mode);

    lua_pushboolean (L, res);
    return 1;
}

static const struct luaL_Reg context_methods [] = {
    {"__tostring", l_context_tostring},
    {"__gc", l_context_destroy},
    {"__newindex", l_context_newindex},
    {"get_geometry", l_context_geometry},
    {"rendering_mode", l_context_rendering_mode},
    {NULL, NULL}
};

static int l_light_manager_tostring (lua_State *L)
{
    struct vox_light_manager **data = luaL_checkudata (L, 1, LIGHT_MANAGER_META);
    struct vox_light_manager *light_manager = *data;

    lua_pushfstring (L, "<Light manager, %d shadowless lights>",
                     vox_shadowless_lights_number (light_manager));
    return 1;
}

static int l_light_manager_len (lua_State *L)
{
    struct vox_light_manager **data = luaL_checkudata (L, 1, LIGHT_MANAGER_META);
    struct vox_light_manager *light_manager = *data;

    lua_pushinteger (L, vox_shadowless_lights_number (light_manager));
    return 1;
}

static int l_insert_shadowless_light (lua_State *L)
{
    vox_dot center, color;
    float radius;

    struct vox_light_manager **data = luaL_checkudata (L, 1, LIGHT_MANAGER_META);
    struct vox_light_manager *light_manager = *data;
    READ_DOT (center, 2);
    radius = luaL_checknumber (L, 3);
    READ_DOT (color, 4);

    int res = vox_insert_shadowless_light (light_manager, center, radius, color);
    lua_pushboolean (L, res);

    return 1;
}

static int l_delete_shadowless_light (lua_State *L)
{
    vox_dot center;
    float radius;

    struct vox_light_manager **data = luaL_checkudata (L, 1, LIGHT_MANAGER_META);
    struct vox_light_manager *light_manager = *data;
    READ_DOT (center, 2);
    radius = luaL_checknumber (L, 3);

    int res = vox_delete_shadowless_light (light_manager, center, radius);
    lua_pushboolean (L, res);

    return 1;
}

static int l_delete_shadowless_lights (lua_State *L)
{
    struct vox_light_manager **data = luaL_checkudata (L, 1, LIGHT_MANAGER_META);
    struct vox_light_manager *light_manager = *data;

    vox_delete_shadowless_lights (light_manager);

    return 0;
}

static int l_set_ambient_light (lua_State *L)
{
    vox_dot color;
    struct vox_light_manager **data = luaL_checkudata (L, 1, LIGHT_MANAGER_META);
    struct vox_light_manager *light_manager = *data;
    READ_DOT (color, 2);

    int res = vox_set_ambient_light (light_manager, color);
    lua_pushboolean (L, res);

    return 1;
}

static const struct luaL_Reg light_manager_methods [] = {
    {"__tostring", l_light_manager_tostring},
    {"__len", l_light_manager_len},
    {"insert_shadowless_light", l_insert_shadowless_light},
    {"delete_shadowless_light", l_delete_shadowless_light},
    {"delete_shadowless_lights", l_delete_shadowless_lights},
    {"set_ambient_light", l_set_ambient_light},
    {NULL, NULL}
};

static int l_new_fps_controller (lua_State *L)
{
    unsigned int fps = luaL_checkinteger (L, 1);
    vox_fps_controller_t *data = lua_newuserdata (L, sizeof (vox_fps_controller_t));
    luaL_getmetatable (L, FPS_CTRL_META);
    lua_setmetatable (L, -2);
    *data = vox_make_fps_controller (fps);

    return 1;
}

static int l_fps_controller_to_string (lua_State *L)
{
    lua_pushfstring (L, "<fps controller at address %p>", lua_topointer (L, 1));
    return 1;
}

static int l_fps_controller_gc (lua_State *L)
{
    vox_fps_controller_t *data = luaL_checkudata (L, 1, FPS_CTRL_META);
    vox_destroy_fps_controller (*data);

    return 0;
}

static int l_fps_controller_delay (lua_State *L)
{
    vox_fps_controller_t *data = luaL_checkudata (L, 1, FPS_CTRL_META);
    vox_fps_controller_t fps_controller = *data;

    struct vox_fps_info fps_info = fps_controller ();
    int updated = vox_fpsstatus_updated (fps_info.status);
    int fps = vox_fpsstatus_fps (fps_info.status);
    Uint32 frame_time = fps_info.frame_time;

    lua_pushinteger (L, fps);
    lua_pushboolean (L, updated);
    lua_pushinteger (L, frame_time);

    return 3;
}

static const struct luaL_Reg fps_controller_methods [] = {
    {"__tostring", l_fps_controller_to_string},
    {"__gc", l_fps_controller_gc},
    {"delay", l_fps_controller_delay},
    {NULL, NULL}
};

static const struct luaL_Reg voxrnd [] = {
    {"camera", new_camera},
    {"cd", new_cd},
    {"fps_controller", l_new_fps_controller},
    {NULL, NULL}
};

static const struct {
    char *name;
    unsigned int mode;
} rnd_modes [] = {
    {"best", VOX_QUALITY_BEST},
    {"adaptive", VOX_QUALITY_ADAPTIVE},
    {"fast", VOX_QUALITY_FAST},
    {"raymerge", VOX_QUALITY_ADAPTIVE | VOX_QUALITY_RAY_MERGE},
    {"accurate_raymerge", VOX_QUALITY_ADAPTIVE | VOX_QUALITY_RAY_MERGE_ACCURATE}
};

int luaopen_voxrnd (lua_State *L)
{
    luaL_newmetatable(L, CAMERA_META);
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, camera_methods, 0);

    luaL_newmetatable(L, CD_META);
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, cd_methods, 0);

    luaL_newmetatable(L, SCENE_PROXY_META);
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, scene_proxy_methods, 0);

    luaL_newmetatable(L, CONTEXT_META);
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, context_methods, 0);

    luaL_newmetatable(L, LIGHT_MANAGER_META);
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, light_manager_methods, 0);

    luaL_newmetatable(L, FPS_CTRL_META);
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, fps_controller_methods, 0);

    luaL_newlib (L, voxrnd);

    int i;
    int nmodes = sizeof (rnd_modes) / sizeof (rnd_modes[0]);
    lua_createtable (L, nmodes, 0);
    for (i=0; i<nmodes; i++) {
        lua_pushinteger (L, rnd_modes[i].mode);
        lua_setfield (L, -2, rnd_modes[i].name);
    }
    lua_setfield (L, -2, "rendering_modes");

    return 1;
}
