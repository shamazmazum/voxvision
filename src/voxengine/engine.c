#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <voxtrees.h>
#include <voxrnd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <modules.h>
#include <sys/param.h>
#include <dlfcn.h>
#include "engine.h"

struct vox_engine
{
    lua_State *L;
    int width, height;
    const char *script;
};

static void usage()
{
    fprintf (stderr, "Usase: <program> [-w width] [-h height] -s script [rest]\n");
}

static int load_module (lua_State *L, const char *modname)
{
    char path[MAXPATHLEN];
    char init[MAXPATHLEN];

    strcpy (path, LUA_MODULE_PATH);
    strcat (path, modname);
    strcat (path, ".so");

    void *handle = dlopen (path, RTLD_LAZY | RTLD_NODELETE);
    if (handle == NULL)
    {
        fprintf (stderr, "Cannot open lua module %s\n", path);
        return 1;
    }

    strcpy (init, "luaopen_");
    strcat (init, modname);

    void *init_func = dlsym (handle, init);
    if (init_func == NULL)
    {
        fprintf (stderr, "Cannot cannot find initfunction %s\n", init);
        return 1;
    }

    luaL_requiref (L, modname, init_func, 0);
    // Copy table in our environment
    lua_setfield (L, -2, modname);
    return 0;
}

// Function is on top of the stack
static void set_safe_environment (lua_State *L)
{
    lua_getglobal (L, "voxvision");
    // FIXME: _ENV is upvalue #1
    lua_setupvalue (L, -2, 1);
}

static int initialize_lua (struct vox_engine *engine)
{
    lua_State *L = luaL_newstate ();
    engine->L = L;
    int res;

    luaL_openlibs (L);
    // Environment is on top of the stack
    lua_newtable (L);
    // Copy environment in global variable
    lua_pushvalue (L, -1);
    lua_setglobal (L, "voxvision");

    if (load_module (L, "voxtrees")) return 1;
    if (load_module (L, "voxrnd")) return 1;

    // Also add some safe functions
    lua_getglobal (L, "print");
    lua_setfield (L, -2, "print");

    lua_getglobal (L, "ipairs");
    lua_setfield (L, -2, "ipairs");

    lua_getglobal (L, "pairs");
    lua_setfield (L, -2, "pairs");

    if (luaL_loadfile (L, engine->script))
    {
        fprintf (stderr, "Error loading script %s\n", engine->script);
        return 1;
    }
    set_safe_environment (L);

    if ((res = lua_pcall (L, 0, 0, 0)))
    {
        fprintf (stderr,
                 "Error executing script %s\n"
                 "%s\n",
                 engine->script,
                 lua_tostring (L, -1));
        return 1;
    }
    return 0;
}

struct vox_engine* vox_create_engine (int *argc, char **argv[])
{
    struct vox_engine *engine;
    int ch, width = 800, height = 600;
    const char *script = NULL;

    while ((ch = getopt (*argc, *argv, "w:h:s:")) != -1)
    {
        switch (ch)
        {
        case 'w':
            width = strtol (optarg, NULL, 10);
            break;
        case 'h':
            height = strtol (optarg, NULL, 10);
            break;
        case 's':
            script = optarg;
            break;
        case '?':
        default:
            usage();
            return NULL;
        }
    }

    *argc -= optind;
    *argv += optind;

    if (script == NULL)
    {
        usage ();
        return NULL;
    }

    engine = malloc (sizeof (struct vox_engine));
    engine->width = width;
    engine->height = height;
    engine->script = script;
    engine->L = NULL;

    if (initialize_lua (engine)) goto bad;

    return engine;

bad:
    vox_destroy_engine (engine);
    return NULL;
}

void vox_destroy_engine (struct vox_engine *engine)
{
    if (engine->L != NULL) lua_close (engine->L);
    free (engine);
}
