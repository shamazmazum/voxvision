#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <voxtrees.h>
#include <stdlib.h>
#include <strings.h>
#include <modules.h>

static int newtree (lua_State *L)
{
    struct nodedata *data = lua_newuserdata (L, sizeof (struct nodedata));
    data->node = NULL;
    luaL_getmetatable (L, "voxtrees.vox_node");
    lua_setmetatable (L, -2);
    return 1;
}

static int newdenseleaf (lua_State *L)
{
    struct vox_box *arg = luaL_checkudata (L, 1, "voxtrees.vox_box");
    struct vox_box box;

    // Fix alignment
    memcpy (&box, arg, sizeof (struct vox_box));

    newtree (L);
    struct nodedata *data = luaL_checkudata (L, -1, "voxtrees.vox_node");
    data->node = vox_make_dense_leaf (&box);
    return 1;
}

static int destroytree (lua_State *L)
{
    struct nodedata *data = luaL_checkudata (L, 1, "voxtrees.vox_node");
    vox_destroy_tree (data->node);
    data->node = NULL;
    return 0;
}

static int counttree (lua_State *L)
{
    struct nodedata *data = luaL_checkudata (L, 1, "voxtrees.vox_node");
    lua_pushinteger (L, vox_voxels_in_tree (data->node));
    return 1;
}

static int inserttree (lua_State *L)
{
    struct nodedata *data = luaL_checkudata (L, 1, "voxtrees.vox_node");
    float *dot = luaL_checkudata (L, 2, "voxtrees.vox_dot");
    vox_dot copy;

    // Fix alignemnt
    memcpy (copy, dot, sizeof(vox_dot));
    int res = vox_insert_voxel (&(data->node), copy);
    lua_pushinteger (L, res);
    return 1;
}

static int rebuildtree (lua_State *L)
{
    struct nodedata *data = luaL_checkudata (L, 1, "voxtrees.vox_node");
    struct vox_node *newtree = vox_rebuild_tree (data->node);
    vox_destroy_tree (data->node);
    data->node = newtree;
    return 0;
}

static int deletetree (lua_State *L)
{
    struct nodedata *data = luaL_checkudata (L, 1, "voxtrees.vox_node");
    float *dot = luaL_checkudata (L, 2, "voxtrees.vox_dot");
    vox_dot copy;

    // Fix alignemnt
    memcpy (copy, dot, sizeof(vox_dot));
    int res = vox_delete_voxel (&(data->node), copy);
    lua_pushinteger (L, res);
    return 1;
}

static int printtree (lua_State *L)
{
    struct nodedata *data = luaL_checkudata (L, 1, "voxtrees.vox_node");
    lua_pushfstring (L, "<tree, %d voxels>",
                     vox_voxels_in_tree (data->node));
    return 1;
}

// FIXME: use of lua_newuserdata()
static int bbtree (lua_State *L)
{
    struct nodedata *arg = luaL_checkudata (L, 1, "voxtrees.vox_node");
    struct vox_box bb;
    struct vox_box *data = lua_newuserdata (L, sizeof (struct vox_box));

    luaL_getmetatable (L, "voxtrees.vox_box");
    lua_setmetatable (L, -2);

    vox_bounding_box (arg->node, &bb);
    memcpy (data, &bb, sizeof(struct vox_box));
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
    {NULL, NULL}
};

static int newdot (lua_State *L)
{
    float x = luaL_checknumber (L, 1);
    float y = luaL_checknumber (L, 2);
    float z = luaL_checknumber (L, 3);
    float *dot = lua_newuserdata (L, sizeof (vox_dot));
    luaL_getmetatable (L, "voxtrees.vox_dot");
    lua_setmetatable (L, -2);
    dot[0] = x; dot[1] = y; dot[2] = z;
    return 1;
}

static int printdot (lua_State *L)
{
    float *dot = luaL_checkudata (L, 1, "voxtrees.vox_dot");
    lua_pushfstring (L, "<dot [%f,%f,%f]>",
                     dot[0], dot[1], dot[2]);
    return 1;
}

static const struct luaL_Reg dot_methods [] = {
    {"__tostring", printdot},
    {NULL, NULL}
};

static int newbox (lua_State *L)
{
    float *min = luaL_checkudata (L, 1, "voxtrees.vox_dot");
    float *max = luaL_checkudata (L, 2, "voxtrees.vox_dot");
    struct vox_box *data = lua_newuserdata (L, sizeof (struct vox_box));
    memcpy (data->min, min, sizeof(vox_dot));
    memcpy (data->max, max, sizeof(vox_dot));
    luaL_getmetatable (L, "voxtrees.vox_box");
    lua_setmetatable (L, -2);
    return 1;
}

static int printbox (lua_State *L)
{
    struct vox_box *data = luaL_checkudata (L, 1, "voxtrees.vox_box");
    lua_pushfstring (L, "<box [%f,%f,%f]-[%f,%f,%f]>",
                     data->min[0], data->min[1], data->min[2],
                     data->max[0], data->max[1], data->max[2]);
    return 1;
}

static const struct luaL_Reg box_methods [] = {
    {"__tostring", printbox},
    {NULL, NULL}
};

struct dotset {
    vox_dot *array;
    unsigned int length, max_length;
};

static int newdotset (lua_State *L)
{
    unsigned int len = luaL_checkunsigned (L, 1);
    struct dotset *set = lua_newuserdata (L, sizeof (struct dotset));
    luaL_getmetatable (L, "voxtrees.dotset");
    lua_setmetatable (L, -2);
    set->length = 0;
    set->max_length = len;
    set->array = aligned_alloc (16, sizeof (vox_dot)*len);

    return 1;
}

static int printdotset (lua_State *L)
{
    struct dotset *set = luaL_checkudata (L, 1, "voxtrees.dotset");
    lua_pushfstring (L, "<dot array, %d dots>", set->length);

    return 1;
}

static int destroydotset (lua_State *L)
{
    struct dotset *set = luaL_checkudata (L, 1, "voxtrees.dotset");
    free (set->array);

    return 0;
}

static int lengthdotset (lua_State *L)
{
    struct dotset *set = luaL_checkudata (L, 1, "voxtrees.dotset");
    lua_pushinteger (L, set->length);

    return 1;
}

static int pushdotset (lua_State *L)
{
    struct dotset *set = luaL_checkudata (L, 1, "voxtrees.dotset");
    float *dot = luaL_checkudata (L, 2, "voxtrees.vox_dot");
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
    struct dotset *set = luaL_checkudata (L, 1, "voxtrees.dotset");
    newtree (L);

    struct nodedata *data = luaL_checkudata (L, -1, "voxtrees.vox_node");
    data->node = vox_make_tree (set->array, set->length);

    return 1;
}

static int voxelsize (lua_State *L)
{
    float *voxel = luaL_checkudata (L, 1, "voxtrees.vox_dot");
    memcpy (vox_voxel, voxel, sizeof (vox_dot));
    return 0;
}

static int read_raw_data (lua_State *L)
{
    const char *filename = luaL_checkstring (L, 1);
    float *dim = luaL_checkudata (L, 2, "voxtrees.vox_dot");
    unsigned int samplesize = luaL_checkinteger (L, 3);
    unsigned int threshold = luaL_checkinteger (L, 4);
    const char *errorstr;
    unsigned int d[3];
    d[0] = dim[0]; d[1] = dim[1]; d[2] = dim[2];
    int res;

    struct vox_node *tree = vox_read_raw_data (filename, d, samplesize, threshold, &errorstr);
    if (tree != NULL)
    {
        res = 1;
        newtree (L);
        struct nodedata *data = luaL_checkudata (L, -1, "voxtrees.vox_node");
        data->node = tree;
    }
    else
    {
        res = 2;
        lua_pushnil (L);
        lua_pushstring (L, errorstr);
    }

    return res;
}

static const struct luaL_Reg voxtrees [] = {
    {"tree", newtree},
    {"dot", newdot},
    {"dotset", newdotset},
    {"boxtree", newdenseleaf},
    {"settree", newsettree},
    {"box", newbox},
    {"voxelsize", voxelsize},
    {"read_raw_data", read_raw_data},
    {NULL, NULL}
};

int luaopen_voxtrees (lua_State *L)
{
    luaL_newmetatable(L, "voxtrees.vox_node");
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, tree_methods, 0);

    luaL_newmetatable(L, "voxtrees.vox_dot");
    luaL_setfuncs (L, dot_methods, 0);

    luaL_newmetatable(L, "voxtrees.vox_box");
    luaL_setfuncs (L, box_methods, 0);

    luaL_newmetatable(L, "voxtrees.dotset");
    lua_pushvalue (L, -1);
    lua_setfield (L, -2, "__index");
    luaL_setfuncs (L, dotset_methods, 0);

    luaL_newlib (L, voxtrees);
    return 1;
}
