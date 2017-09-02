#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <voxrnd.h>
#include <stdlib.h>
#include "../modules.h"

static int get_position (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    vox_dot position;
    camera->iface->get_position (camera->camera, position);
    WRITE_DOT (position);

    return 1;
}

static int set_position (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    vox_dot position;
    READ_DOT (position, 2);
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
    vox_dot angles;
    READ_DOT (angles, 2);
    camera->iface->set_rot_angles (camera->camera, angles);

    return 0;
}

static int rotate_camera (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    vox_dot delta;
    READ_DOT (delta, 2);
    camera->iface->rotate_camera (camera->camera, delta);

    return 0;
}

static int move_camera (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    vox_dot delta;
    READ_DOT (delta, 2);
    camera->iface->move_camera (camera->camera, delta);

    return 0;
}

static int look_at (lua_State *L)
{
    struct cameradata *camera = luaL_checkudata (L, 1, "voxrnd.camera");
    vox_dot coord;
    READ_DOT (coord, 2);
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

static int new_cd (lua_State *L)
{
    struct cddata *data = lua_newuserdata (L, sizeof (struct cddata));
    data->cd = vox_make_cd ();
    luaL_getmetatable (L, "voxrnd.cd");
    lua_setmetatable (L, -2);
    return 1;
}

static int printcd (lua_State *L)
{
    lua_pushfstring (L, "<collision detector>");
    return 1;
}

static int destroycd (lua_State *L)
{
    struct cddata *cd = luaL_checkudata (L, 1, "voxrnd.cd");
    free (cd->cd);

    return 0;
}

static int cd_attach_camera (lua_State *L)
{
    struct cddata *cd = luaL_checkudata (L, 1, "voxrnd.cd");
    struct cameradata *camera = luaL_checkudata (L, 2, "voxrnd.camera");
    float radius = luaL_checknumber (L, 3);

    vox_cd_attach_camera (cd->cd, camera->camera, radius);

    return 0;
}

static int cd_gravity (lua_State *L)
{
    struct cddata *cd = luaL_checkudata (L, 1, "voxrnd.cd");
    vox_dot gravity;
    READ_DOT (gravity, 2);

    vox_cd_gravity (cd->cd, gravity);

    return 0;
}

static const struct luaL_Reg cd_methods [] = {
    {"__gc", destroycd},
    {"__tostring", printcd},
    {"attach_camera", cd_attach_camera},
    {"gravity", cd_gravity},
    {NULL, NULL}
};

static const struct luaL_Reg voxrnd [] = {
    {"simple_camera", new_simple_camera},
    {"distorted_camera", new_distorted_camera},
    {"cd", new_cd},
    {NULL, NULL}
};

int luaopen_voxrnd (lua_State *L)
{
    luaL_newmetatable(L, "voxrnd.camera");
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, camera_methods, 0);

    luaL_newmetatable(L, "voxrnd.cd");
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, cd_methods, 0);

    luaL_newlib (L, voxrnd);
    return 1;
}
