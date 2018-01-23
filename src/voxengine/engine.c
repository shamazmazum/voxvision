#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <dlfcn.h>
#include <assert.h>

#include "modules.h"
#include "engine.h"

static int engine_key;

static int engine_panic (lua_State *L)
{
    struct vox_engine **data;

    fprintf (stderr,
             "%s\n"
             "Unhandeled error, quiting. Doublecheck your scripts\n",
             lua_tostring (L, -1));

    lua_pushlightuserdata (L, &engine_key);
    lua_gettable (L, LUA_REGISTRYINDEX);
    data = lua_touserdata (L, -1);

    vox_destroy_engine (*data);
    exit(0);
}

// Load module, but do not add returned table to environment
static void load_module_restricted (lua_State *L, const char *modname)
{
    char path[MAXPATHLEN];
    char init[MAXPATHLEN];

    if (snprintf (path, MAXPATHLEN, "%s%s.so", VOX_MODULE_PATH, modname) >= MAXPATHLEN)
        luaL_error (L, "Module path name is too long");

    void *handle = dlopen (path, RTLD_LAZY | RTLD_NODELETE);
    if (handle == NULL) luaL_error (L, "Cannot open lua module %s", path);

    if (snprintf (init, MAXPATHLEN, "luaopen_%s", modname) >= MAXPATHLEN)
        luaL_error (L, "Module path name is too long");

    void *init_func = dlsym (handle, init);
    if (init_func == NULL) luaL_error (L, "Cannot cannot find init function %s", init);

    luaL_requiref (L, modname, init_func, 0);
}

static void load_module (lua_State *L, const char *modname)
{
    load_module_restricted (L, modname);
    // Copy table in our environment
    lua_setfield (L, -2, modname);
}

static void load_lua_module (lua_State *L, const char *modname)
{
    char path[MAXPATHLEN];

    if (snprintf (path, MAXPATHLEN, "%s%s.lua", VOX_MODULE_PATH, modname) >= MAXPATHLEN)
        luaL_error (L, "Module path name is too long");

    if (luaL_loadfile (L, path) || lua_pcall (L, 0, 1, 0))
        luaL_error (L, "Cannot load module %s: %s", modname, lua_tostring (L, -1));

    // Copy table in our environment
    lua_setfield (L, -2, modname);
}

static void set_safe_environment (lua_State *L)
{
    // Function to be executed is on top of the stack
    lua_getglobal (L, "voxvision");
    // FIXME: _ENV is upvalue #1
    lua_setupvalue (L, -2, 1);
}

static void prepare_safe_environment (lua_State *L)
{
    // Our environment is on top of the stack
#include <safe_environment.h>
}

static void initialize_lua (struct vox_engine *engine)
{
    lua_State *L = luaL_newstate ();
    engine->L = L;

    luaL_openlibs (L);
    // Put engine in the registry and set panic handler
    lua_pushlightuserdata (L, &engine_key);
    struct vox_engine **data = lua_newuserdata (L, sizeof (struct vox_engine*));
    *data = engine;
    lua_settable (L, LUA_REGISTRYINDEX);
    lua_atpanic (L, engine_panic);

    // Place our safe environment is on top of the stack
    lua_newtable (L);
    // Copy environment in global variable
    lua_pushvalue (L, -1);
    lua_setglobal (L, "voxvision");

    // Load C modules
    load_module (L, "voxtrees");
    load_module (L, "voxrnd");

    // Also add some safe functions
    prepare_safe_environment (L);

    // And load lua modules
    load_lua_module (L, "voxutils");

    // Remove the environment from the stack, it is in "voxvision" global now
    lua_pop (L, 1);
    assert (lua_gettop (engine->L) == 0);
}

static int execute_init (struct vox_engine *engine)
{
    lua_State *L = engine->L;

    lua_getglobal (L, "voxvision");
    lua_getfield (L, -1, "init");
    lua_remove (L, -2);

    if (!lua_isfunction (L, -1))
        luaL_error (L, "init is not a function: %s", lua_tostring (L, -1));

    set_safe_environment (L);

    // Context is on top of the stack
    lua_pushvalue (L, 1);
    if (lua_pcall (L, 1, 1, 0))
        luaL_error (L, "Error executing init function: %s", lua_tostring (L, -1));

    int status = lua_isnil (L, -1);
    lua_pop (L, 1);

    // Check that we have only the context on the stack
    assert (lua_gettop (engine->L) == 1);

    /* Copy context group. */
    struct context_data *data = luaL_checkudata (L, 1, CONTEXT_META);
    engine->rendering_queue = data->rendering_queue;

    /* Check that we have the world properly set up */
    if (data->context->scene == NULL ||
        data->context->camera == NULL)
        luaL_error (L, "World is not properly set up");

    /*
     * Did the engine was created only for debugging purposes?
     * If the answer is yes, init() must return nil.
     */
    return status;
}

static void engine_load_script (struct vox_engine *engine, const char *script)
{
    lua_State *L = engine->L;

    if (luaL_loadfile (L, script))
        luaL_error (L, "Error loading script %s: %s", script,
                    lua_tostring (L, -1));
    set_safe_environment (L);

    if (lua_pcall (L, 0, 0, 0))
        luaL_error (L,
                    "Error executing script %s\n"
                    "%s",
                    script,
                    lua_tostring (L, -1));

    // Check that the stack is empty
    assert (lua_gettop (engine->L) == 0);
}

static vox_engine_status execute_tick (struct vox_engine *engine)
{
    lua_State *L = engine->L;
    vox_engine_status res;

    lua_getglobal (L, "voxvision");
    lua_getfield (L, -1, "tick");

    if (!lua_isnil (L, -1))
    {
        // Assume that tick is a function
        set_safe_environment (L);

        // Copy the "world" table
        lua_pushvalue (L, 1);
        lua_pushnumber (L, SDL_GetTicks());
        if (lua_pcall (L, 2, 1, 0))
            luaL_error (L, "error executing tick function: %s", lua_tostring (L, -1));
        res = lua_toboolean (L, -1);
    }
    else
    {
        // Do very basic SDL event handling here
        SDL_Event event;
        res = 1;

        if (SDL_PollEvent (&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                res = 0;
                break;
            default:
                ;
            }
        }
    }
    lua_pop (L, 2);

    return res;
}

void initialize_arguments (const struct vox_engine *engine,
                           int nargs, char * const arguments[])
{
    lua_State *L = engine->L;
    int i;

    if (nargs != 0) {
        lua_getglobal (L, "voxvision");
        lua_newtable (L);
        for (i=0; i<nargs; i++) {
            lua_pushstring (L, arguments[i]);
            lua_seti (L, -2, i+1);
        }

        lua_setfield (L, -2, "arg");
        // Pop voxvision table from the stack
        lua_pop (L, 1);
    }

    assert (lua_gettop (L) == 0);
}

struct vox_engine* vox_create_engine (int width, int height, const char *script,
                                      int nargs, char * const arguments[])
{
    struct vox_engine *engine;

    /* Arguments sanity check */
    if (script == NULL) return NULL;
    if ((nargs == 0 && arguments != NULL) ||
        (nargs != 0 && arguments == NULL))
        return NULL;
 
    engine = malloc (sizeof (struct vox_engine));
    memset (engine, 0, sizeof (struct vox_engine));
    engine->width = width;
    engine->height = height;

    initialize_lua (engine);
    initialize_arguments (engine, nargs, arguments);
    engine_load_script (engine, script);

    // Init SDL
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        fprintf (stderr, "Cannot init SDL: %s\n", SDL_GetError());
        goto bad;
    }

    engine->ctx = vox_make_context_and_window (engine->width, engine->height);
    if (engine->ctx == NULL)
    {
        fprintf (stderr, "Cannot create the context: %s\n", SDL_GetError());
        goto bad;
    }

    // Create context on top of lua stack
    struct context_data *data = lua_newuserdata (engine->L, sizeof (struct context_data));
    luaL_getmetatable (engine->L, CONTEXT_META);
    lua_setmetatable (engine->L, -2);
    data->context = engine->ctx;

    if (execute_init (engine)) {
        // Was called only for debugging
        goto bad;
    }

    return engine;

bad:
    vox_destroy_engine (engine);
    return NULL;
}

vox_engine_status vox_engine_tick (struct vox_engine *engine)
{
    vox_engine_status res;
    /*
     * If we have a queue associated with the tree (this is so if we use
     * thread-safe scene proxy in the world table), enqueue the rendering
     * operation to that queue and wait for its completion. This will assure us
     * that the tree is in consistent state while the rendering is performed.
     */
    dispatch_sync (engine->rendering_queue, ^{
            vox_render (engine->ctx);
        });
    vox_redraw (engine->ctx);

    res = execute_tick (engine);
    assert (lua_gettop (engine->L) == 1);
    return res;
}

void vox_destroy_engine (struct vox_engine *engine)
{
    if (engine->L != NULL) lua_close (engine->L);
    if (SDL_WasInit(0)) SDL_Quit();
    free (engine);
}
