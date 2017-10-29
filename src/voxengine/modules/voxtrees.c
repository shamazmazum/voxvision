#include <sys/param.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <voxtrees.h>
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

static const struct luaL_Reg tree_methods [] = {
    {"__len", counttree},
    {"__tostring", printtree},
    {"__gc", destroytree},
    {"insert", inserttree},
    {"delete", deletetree},
    {"rebuild", rebuildtree},
    {"bounding_box", bbtree},
    {NULL, NULL}
};

struct dotset {
    vox_dot *array;
    unsigned int length, max_length;
};

static int newdotset (lua_State *L)
{
    unsigned int len = luaL_checkinteger (L, 1);
    struct dotset *set = lua_newuserdata (L, sizeof (struct dotset));
    luaL_getmetatable (L, DOTSET_META);
    lua_setmetatable (L, -2);
    set->length = 0;
    set->max_length = len;
    set->array = aligned_alloc (16, sizeof (vox_dot)*len);

    return 1;
}

static int printdotset (lua_State *L)
{
    struct dotset *set = luaL_checkudata (L, 1, DOTSET_META);
    lua_pushfstring (L, "<dot array, %d dots>", set->length);

    return 1;
}

static int destroydotset (lua_State *L)
{
    struct dotset *set = luaL_checkudata (L, 1, DOTSET_META);
    free (set->array);

    return 0;
}

static int lengthdotset (lua_State *L)
{
    struct dotset *set = luaL_checkudata (L, 1, DOTSET_META);
    lua_pushinteger (L, set->length);

    return 1;
}

static int pushdotset (lua_State *L)
{
    struct dotset *set = luaL_checkudata (L, 1, DOTSET_META);
    vox_dot dot;
    READ_DOT (dot, 2);
    int argc = 0;

    if (set->length == set->max_length)
    {
        lua_pushstring (L, "set is full");
        argc = 1;
    }
    else memcpy (set->array[set->length++], dot, sizeof(vox_dot));

    return argc;
}

static const struct luaL_Reg dotset_methods [] = {
    {"__tostring", printdotset},
    {"__gc", destroydotset},
    {"__len", lengthdotset},
    {"push", pushdotset},
    {NULL, NULL}
};

static int newsettree (lua_State *L)
{
    struct dotset *set = luaL_checkudata (L, 1, DOTSET_META);
    newtree (L);

    struct vox_node **data = luaL_checkudata (L, -1, TREE_META);
    *data = vox_make_tree (set->array, set->length);

    return 1;
}

static int voxelsize (lua_State *L)
{
    vox_dot voxel;
    READ_DOT (voxel, 1);
    memcpy (vox_voxel, voxel, sizeof (vox_dot));
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
    {"dotset", newdotset},
    {"boxtree", newdenseleaf},
    {"settree", newsettree},
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

    luaL_newmetatable(L, DOTSET_META);
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, dotset_methods, 0);

    luaL_newlib (L, voxtrees);
    return 1;
}
