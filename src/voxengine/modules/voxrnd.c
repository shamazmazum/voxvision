#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <voxrnd.h>
#include <voxtrees.h>
#include <stdlib.h>
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

static int cd_gravity (lua_State *L)
{
    struct vox_cd **cd = luaL_checkudata (L, 1, CD_META);
    vox_dot gravity;
    READ_DOT (gravity, 2);

    vox_cd_gravity (*cd, gravity);

    return 0;
}

static const struct luaL_Reg cd_methods [] = {
    {"__gc", destroycd},
    {"__tostring", printcd},
    {"attach_camera", cd_attach_camera},
    {"gravity", cd_gravity},
    {NULL, NULL}
};

static int l_scene_proxy (lua_State *L)
{
    /*
     * Because this object is associated engine-global context, it's a good idea
     * to implement it like a singleton.
     */
    struct vox_node **ndata = luaL_checkudata (L, 1, TREE_META);
    luaL_getmetatable (L, SCENE_PROXY_META);
    lua_getfield (L, -1, "__self");
    struct scene_proxydata *data = luaL_testudata (L, -1, SCENE_PROXY_META);
    if (data != NULL) {
        /* The object is already created, just set a new tree. */
        data->tree = *ndata;

        /* Updata lua reference */
        lua_pushvalue (L, 1);
        lua_setfield (L, -3, "__tree");
    } else {
        /* Pop __self */
        lua_pop (L, 1);
        data = lua_newuserdata (L, sizeof (struct scene_proxydata));
        /* Copy metatable */
        lua_pushvalue (L, -2);
        lua_setmetatable (L, -2);

        /*
         * Fill C-side internal fields.
         */
        data->tree = *ndata;
        data->scene_group = dispatch_group_create ();
        data->scene_sync_queue = dispatch_queue_create ("scene operations", 0);

        /*
         * Store reference to the tree in the proxy's metatable to prevent GC from
         * destroying it.
         */

        /* Copy tree */
        lua_pushvalue (L, 1);
        lua_setfield (L, -3, "__tree");

        /* Copy self */
        lua_pushvalue (L, -1);
        lua_setfield (L, -3, "__self");
    }
    return 1;
}

static int l_scene_proxy_destroy (lua_State *L)
{
    struct scene_proxydata *data = luaL_checkudata (L, 1, SCENE_PROXY_META);
    dispatch_release (data->scene_sync_queue);
    dispatch_release (data->scene_group);

    return 0;
}

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
    {"__gc", l_scene_proxy_destroy},
    {"rebuild", l_scene_proxy_rebuild},
    {"insert", l_scene_proxy_insert},
    {"delete", l_scene_proxy_delete},
    {"ray_intersection", l_scene_proxy_ray_intersection},
    {NULL, NULL}
};

static int l_sphere (lua_State *L)
{
    struct vox_sphere **data = lua_newuserdata (L, sizeof (struct vox_sphere*));
    /*
     * We cannot hold sphere in userdata itself because it does not guarantee
     * alignment.
     */
    struct vox_sphere *sphere = vox_alloc (sizeof (struct vox_sphere));
    *data = sphere;
    luaL_getmetatable (L, SPHERE_META);
    lua_setmetatable (L, -2);

    READ_DOT (sphere->center, 1);
    sphere->radius = luaL_checknumber (L, 2);

    return 1;
}

static int l_sphere_destroy (lua_State *L)
{
    struct vox_sphere **data = luaL_checkudata (L, 1, SPHERE_META);
    free (*data);
    *data = NULL;

    return 0;
}

static int l_sphere_tostring (lua_State *L)
{
    struct vox_sphere **data = luaL_checkudata (L, 1, SPHERE_META);
    struct vox_sphere *sphere = *data;

    lua_pushfstring (L, "<Sphere center=<%f, %f, %f>, radius=%f>",
                     sphere->center[0], sphere->center[1], sphere->center[2],
                     sphere->radius);
    return 1;
}

static const struct luaL_Reg sphere_methods [] = {
    {"__tostring", l_sphere_tostring},
    {"__gc", l_sphere_destroy},
    {NULL, NULL}
};

static const struct luaL_Reg voxrnd [] = {
    {"camera", new_camera},
    {"cd", new_cd},
    {"scene_proxy", l_scene_proxy},
    {"sphere", l_sphere},
    {NULL, NULL}
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

    luaL_newmetatable(L, SPHERE_META);
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, sphere_methods, 0);

    luaL_newlib (L, voxrnd);
    return 1;
}
