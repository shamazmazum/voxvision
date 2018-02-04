#include <sys/param.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <voxtrees-ng.h>
#include "../modules.h"

#include <stdlib.h>

static int newtree (lua_State *L)
{
    struct vox_node **data = lua_newuserdata (L, sizeof (struct vox_node**));
    *data = NULL;
    luaL_getmetatable (L, TREE_META);
    lua_setmetatable (L, -2);
    return 1;
}

static int newdenseleaf (lua_State *L)
{
    struct vox_box box;
    READ_DOT (box.min, 1);
    READ_DOT (box.max, 2);

    newtree (L);
    struct vox_node **data = luaL_checkudata (L, -1, TREE_META);
    *data = vox_make_dense_leaf (&box);
    return 1;
}

static int destroytree (lua_State *L)
{
    struct vox_node **data = luaL_checkudata (L, 1, TREE_META);
    vox_destroy_tree (*data);
    *data = NULL;
    return 0;
}

static int counttree (lua_State *L)
{
    struct vox_node **data = luaL_checkudata (L, 1, TREE_META);
    lua_pushinteger (L, vox_voxels_in_tree (*data));
    return 1;
}

static int inserttree (lua_State *L)
{
    struct vox_node **data = luaL_checkudata (L, 1, TREE_META);
    vox_dot dot;

    READ_DOT (dot, 2);
    int res = vox_insert_voxel (data, dot);
    lua_pushboolean (L, res);
    return 1;
}

static int rebuildtree (lua_State *L)
{
    struct vox_node **data = luaL_checkudata (L, 1, TREE_META);
    struct vox_node *newtree = vox_rebuild_tree (*data);
    vox_destroy_tree (*data);
    *data = newtree;
    return 0;
}

static int deletetree (lua_State *L)
{
    struct vox_node **data = luaL_checkudata (L, 1, TREE_META);
    vox_dot dot;

    READ_DOT (dot, 2);
    int res = vox_delete_voxel (data, dot);
    lua_pushboolean (L, res);
    return 1;
}

static int printtree (lua_State *L)
{
    struct vox_node **data = luaL_checkudata (L, 1, TREE_META);
    lua_pushfstring (L, "<tree %p, %d voxels>", *data,
                     vox_voxels_in_tree (*data));
    return 1;
}

static int bbtree (lua_State *L)
{
    struct vox_node **data = luaL_checkudata (L, 1, TREE_META);
    struct vox_box bb;

    vox_bounding_box (*data, &bb);
    WRITE_DOT (bb.min);
    WRITE_DOT (bb.max);

    return 2;
}

static int l_tree_ray_intersection (lua_State *L)
{
    struct vox_node **data = luaL_checkudata (L, 1, TREE_META);
    vox_dot origin, dir, res;
    READ_DOT (origin, 2);
    READ_DOT (dir, 3);

    const struct vox_node *leaf = vox_ray_tree_intersection (*data, origin, dir, res);
    if (leaf != NULL) WRITE_DOT (res);
    else lua_pushnil (L);

    return 1;
}

static const struct luaL_Reg tree_methods [] = {
    {"__len", counttree},
    {"__tostring", printtree},
    {"__gc", destroytree},
    {"insert", inserttree},
    {"delete", deletetree},
    {"rebuild", rebuildtree},
    {"bounding_box", bbtree},
    {"ray_intersection", l_tree_ray_intersection},
    {NULL, NULL}
};

static int newdotmap (lua_State *L)
{
    lua_geti (L, 1, 1);
    lua_geti (L, 1, 2);
    lua_geti (L, 1, 3);

    unsigned int dim[3];
    dim[0] = luaL_checkinteger (L, 2);
    dim[1] = luaL_checkinteger (L, 3);
    dim[2] = luaL_checkinteger (L, 4);

    struct vox_map_3d **data = lua_newuserdata (L, sizeof (struct vox_map_3d*));
    *data = vox_create_map_3d (dim);

    luaL_getmetatable (L, DOTMAP_META);
    lua_setmetatable (L, -2);

    return 1;
}

static int printdotmap (lua_State *L)
{
    struct vox_map_3d **data = luaL_checkudata (L, 1, DOTMAP_META);
    struct vox_map_3d *map = *data;

    lua_pushfstring (L, "<dot map %ix%ix%i>", map->dim[0], map->dim[1], map->dim[2]);

    return 1;
}

static int destroydotmap (lua_State *L)
{
    struct vox_map_3d **data = luaL_checkudata (L, 1, DOTMAP_META);
    struct vox_map_3d *map = *data;
    vox_destroy_map_3d (map);

    return 0;
}

static int setdotmap (lua_State *L)
{
    struct vox_map_3d **data = luaL_checkudata (L, 1, DOTMAP_META);
    struct vox_map_3d *map = *data;
    int set = lua_toboolean (L, 3);

    lua_geti (L, 2, 1);
    lua_geti (L, 2, 2);
    lua_geti (L, 2, 3);

    unsigned int pos[3];
    pos[0] = luaL_checkinteger (L, 4);
    pos[1] = luaL_checkinteger (L, 5);
    pos[2] = luaL_checkinteger (L, 6);

    int success = 0;
    if (pos[0] < map->dim[0] &&
        pos[1] < map->dim[1] &&
        pos[2] < map->dim[2]) {
        success = 1;
        size_t idx = map->dim[1]*map->dim[2]*pos[0] +
            map->dim[2]*pos[1] + pos[2];
        map->map[idx] = set;
    }

    lua_pushboolean (L, success);
    return 1;
}

static const struct luaL_Reg dotmap_methods [] = {
    {"__tostring", printdotmap},
    {"__gc", destroydotmap},
    {"set", setdotmap},
    {NULL, NULL}
};

static int newmaptree (lua_State *L)
{
    struct vox_map_3d **data = luaL_checkudata (L, 1, DOTMAP_META);
    struct vox_map_3d *map = *data;
    newtree (L);

    struct vox_node **tdata = luaL_checkudata (L, -1, TREE_META);
    *tdata = vox_make_tree (map);

    return 1;
}

static int voxelsize (lua_State *L)
{
    vox_dot voxel;
    READ_DOT (voxel, 1);
    vox_set_voxel (voxel);
    return 0;
}

static int read_raw_data (lua_State *L)
{
    const char *filename = luaL_checkstring (L, 1);
    vox_dot dim;
    READ_DOT (dim, 2);
    unsigned int samplesize = luaL_checkinteger (L, 3);
    const char *errorstr;
    unsigned int d[3];
    d[0] = dim[0]; d[1] = dim[1]; d[2] = dim[2];
    int res;

    struct vox_node *tree = vox_read_raw_data (filename, d, samplesize,
                                               ^(unsigned int sample) {
                                                   lua_pushvalue (L, -1);
                                                   lua_pushinteger (L, sample);
                                                   if (lua_pcall (L, 1, 1, 0))
                                                       luaL_error (L, "Error executing callback: %s",
                                                                   lua_tostring (L, -1));
                                                   int res = lua_toboolean (L, -1);
                                                   lua_pop (L, 1);
                                                   return res;
                                               }, &errorstr);
    if (tree != NULL)
    {
        res = 1;
        newtree (L);
        struct vox_node **data = luaL_checkudata (L, -1, TREE_META);
        *data = tree;
    }
    else
    {
        res = 2;
        lua_pushnil (L);
        lua_pushstring (L, errorstr);
    }

    return res;
}

static int read_raw_data_ranged (lua_State *L)
{
    const char *filename = luaL_checkstring (L, 1);
    vox_dot dim;
    READ_DOT (dim, 2);
    unsigned int samplesize = luaL_checkinteger (L, 3);
    unsigned int min, max;
    if (lua_isnoneornil (L, 4)) min = 1 << (8*samplesize-1);
    else min = luaL_checkinteger (L, 4);
    if (lua_isnoneornil (L, 5)) max = 1<<8*samplesize;
    else max = luaL_checkinteger (L, 5);

    const char *errorstr;
    unsigned int d[3];
    d[0] = dim[0]; d[1] = dim[1]; d[2] = dim[2];
    int res;

    struct vox_node *tree = vox_read_raw_data (filename, d, samplesize,
                                               ^(unsigned int sample) {
                                                   return (sample >= min) ? (sample < max) : 0;
                                               }, &errorstr);
    if (tree != NULL)
    {
        res = 1;
        newtree (L);
        struct vox_node **data = luaL_checkudata (L, -1, TREE_META);
        *data = tree;
    }
    else
    {
        res = 2;
        lua_pushnil (L);
        lua_pushstring (L, errorstr);
    }

    return res;
}

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

static const struct luaL_Reg voxtrees [] = {
    {"tree", newtree},
    {"dotmap", newdotmap},
    {"boxtree", newdenseleaf},
    {"maptree", newmaptree},
    {"voxelsize", voxelsize},
    {"read_raw_data", read_raw_data},
    {"read_raw_data_ranged", read_raw_data_ranged},
    {"find_data_file", find_data_file},
    {NULL, NULL}
};

int luaopen_voxtrees (lua_State *L)
{
    luaL_newmetatable(L, TREE_META);
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, tree_methods, 0);

    luaL_newmetatable(L, DOTMAP_META);
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, dotmap_methods, 0);

    luaL_newlib (L, voxtrees);
    return 1;
}
