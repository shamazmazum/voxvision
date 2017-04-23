#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <voxrnd.h>
#include <stdlib.h>
#include <strings.h>

struct cameradata
{
    struct vox_camera *camera;
    struct vox_camera_interface *iface;
};

static int get_position (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    float *data = lua_newuserdata (L, sizeof (vox_dot));
    vox_dot position;
    luaL_getmetatable (L, "voxtrees.vox_dot");
    lua_setmetatable (L, -2);

    camera->iface->get_position (camera->camera, position);
    memcpy (data, position, sizeof (vox_dot));

    return 1;
}

static int set_position (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    float *data = luaL_checkudata (L, 2, "voxtrees.vox_dot");
    vox_dot position;
    memcpy (position, data, sizeof (vox_dot));

    camera->iface->set_position (camera->camera, position);

    return 0;
}

static int get_fov (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    lua_pushnumber (L, camera->iface->get_fov (camera->camera));

    return 1;
}

static int set_fov (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    float fov = luaL_checknumber (L, 2);
    camera->iface->set_fov (camera->camera, fov);

    return 0;
}

static int set_rot_angles (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    float *data = luaL_checkudata (L, 2, "voxtrees.vox_dot");
    vox_dot angles;
    memcpy (angles, data, sizeof (vox_dot));

    camera->iface->set_rot_angles (camera->camera, angles);

    return 0;
}

static int rotate_camera (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    float *data = luaL_checkudata (L, 2, "voxtrees.vox_dot");
    vox_dot delta;
    memcpy (delta, data, sizeof (vox_dot));

    camera->iface->rotate_camera (camera->camera, delta);

    return 0;
}

static int move_camera (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    float *data = luaL_checkudata (L, 2, "voxtrees.vox_dot");
    vox_dot delta;
    memcpy (delta, data, sizeof (vox_dot));

    camera->iface->move_camera (camera->camera, delta);

    return 0;
}

static int look_at (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    float *data = luaL_checkudata (L, 2, "voxtrees.vox_dot");
    vox_dot coord;
    memcpy (coord, data, sizeof (vox_dot));

    camera->iface->look_at (camera->camera, coord);

    return 0;
}

static int new_simple_camera (lua_State *L)
{
    struct cameradata *data = lua_newuserdata (L, sizeof (struct cameradata));
    data->camera = vox_simple_camera_iface()->construct_camera (NULL);
    data->iface = data->camera->iface;
    luaL_getmetatable (L, "voxrnd.camera");
    lua_setmetatable (L, -2);
    return 1;
}

static int new_distorted_camera (lua_State *L)
{
    struct cameradata *data = lua_newuserdata (L, sizeof (struct cameradata));
    data->camera = vox_distorted_camera_iface()->construct_camera (NULL);
    data->iface = data->camera->iface;
    luaL_getmetatable (L, "voxrnd.camera");
    lua_setmetatable (L, -2);
    return 1;
}

static int destroycamera (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    camera->iface->destroy_camera (camera->camera);

    return 0;
}

static int printcamera (lua_State *L)
{
    lua_pushfstring (L, "<camera>");
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
    {NULL, NULL}
};

static const struct luaL_Reg voxrnd [] = {
    {"simple_camera", new_simple_camera},
    {"distorted_camera", new_distorted_camera},
    {NULL, NULL}
};

int luaopen_voxrnd (lua_State *L)
{
    luaL_newmetatable(L, "voxrnd.camera");
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, camera_methods, 0);

    luaL_newlib (L, voxrnd);
    return 1;
}
