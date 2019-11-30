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

static void set_safe_environment (lua_State *L)
{
    // Function to be executed is on top of the stack
    lua_getglobal (L, "voxvision");
    // FIXME: _ENV is upvalue #1
    lua_setupvalue (L, -2, 1);
}

// FIXME: Only camera can be a vox_object now
static int l_set_property (lua_State *L)
{
    struct vox_object **data = luaL_checkudata (L, 1, CAMERA_META);
    struct vox_object *obj = *data;
    const char *name = luaL_checkstring (L, 2);
    int t = lua_type (L, 3);
    vox_dot dot;
    switch (t) {
    case LUA_TNUMBER:
        obj->iface->set_property_number (obj, name, lua_tonumber (L, 3));
        break;
    case LUA_TTABLE:
        READ_DOT (dot, 3);
        obj->iface->set_property_dot (obj, name, dot);
        break;
    }

    return 0;
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

    // Add some core functions
    lua_pushcfunction (L, l_set_property);
    lua_setfield (L, -2, "set_property");

    char envpath[MAXPATHLEN];
    sprintf (envpath, "%s/loadenvironment.lua", VOX_MODULE_PATH);
    if (luaL_loadfile (L, envpath) || lua_pcall (L, 0, 0, 0)) {
        luaL_error (L, "Cannot load environment script: %s", lua_tostring (L, -1));
    }

    // Remove the environment from the stack, it is in "voxvision" global now
    lua_pop (L, 1);
    assert (lua_gettop (engine->L) == 0);
}

static void execute_init (struct vox_engine *engine)
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
    if (lua_pcall (L, 1, 0, 0))
        luaL_error (L, "Error executing init function: %s", lua_tostring (L, -1));

    if (!(engine->flags & VOX_ENGINE_DEBUG)) {
        /* Copy context group. */
        struct context_data *data = luaL_checkudata (L, 1, CONTEXT_META);
        engine->rendering_queue = data->rendering_queue;

        /* Check that we have the world properly set up */
        lua_getfield (L, 1, "tree");
        lua_getfield (L, 1, "camera");

        if (luaL_testudata (L, -2, SCENE_PROXY_META) == NULL ||
            luaL_testudata (L, -1, CAMERA_META) == NULL)
            luaL_error (L, "World is not properly set up");

        lua_pop (L, 2);
    }

    // Check that we have only the context on the stack
    assert (lua_gettop (engine->L) == 1);
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

/* Lua-side of rendering context */
static void create_lua_context (struct vox_engine *engine)
{
    lua_State *L = engine->L;

    if (engine->flags & VOX_ENGINE_DEBUG) {
        /* In debug mode we just create an empty table for context. */
        lua_newtable (L);
    } else {
        struct context_data *data = lua_newuserdata (L, sizeof (struct context_data));
        luaL_getmetatable (L, CONTEXT_META);
        lua_setmetatable (L, -2);
        data->context = engine->ctx;

        data->rendering_group = dispatch_group_create ();
        data->rendering_queue = dispatch_queue_create ("scene operations", 0);
    }
}

struct vox_engine* vox_create_engine (unsigned int width, unsigned int height,
                                      unsigned int flags,
                                      const char *script,
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
    engine->flags = flags;

    initialize_lua (engine);
    initialize_arguments (engine, nargs, arguments);
    engine_load_script (engine, script);

    /* Do not init SDL in debug mode! */
    if (!(engine->flags & VOX_ENGINE_DEBUG)) {
        // Init SDL
        if (!SDL_WasInit (SDL_INIT_VIDEO) &&
            SDL_Init (SDL_INIT_VIDEO) != 0) {

            fprintf (stderr, "Cannot init SDL video subsystem: %s\n", SDL_GetError());
            goto bad;
        }

        if (!SDL_WasInit (SDL_INIT_TIMER) &&
            SDL_Init (SDL_INIT_TIMER) != 0) {

            fprintf (stderr, "Cannot init SDL timer: %s\n", SDL_GetError());
            goto bad;
        }

        engine->ctx = vox_make_context_and_window (engine->width, engine->height);
        if (engine->ctx == NULL)
        {
            fprintf (stderr, "Cannot create the context: %s\n", SDL_GetError());
            goto bad;
        }
    }

    // Create context on top of lua stack
    create_lua_context (engine);
    execute_init (engine);

    return engine;

bad:
    vox_destroy_engine (engine);
    return NULL;
}

vox_engine_status vox_engine_tick (struct vox_engine *engine)
{
    vox_engine_status res = 0;
    /*
     * If we are in debug mode, do nothing and return 0, meaning that we want to
     * quit.
     */
    if (!(engine->flags & VOX_ENGINE_DEBUG)) {
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
    }
    return res;
}

void vox_destroy_engine (struct vox_engine *engine)
{
    if (engine->L != NULL) lua_close (engine->L);
    if (SDL_WasInit(0)) SDL_Quit();
    free (engine);
}
