Voxvision Manual        {#mainpage}
===============

Voxtrees
--------
### Defining a voxel
Each voxel in the **voxtrees** library is represented by the type `vox_dot`,
which is just an array of 3 single float numbers. This dot type declares a
vertex of a voxel with the minimal coordinates. If you add `vox_voxel` global
variable to this dot, you will get another vertex of your voxel with maximal
coordinates. You can imagine a Cartesian coordinate system with origin `{0,0,0}`
and axes given by equations *x = 0*, *y = 0*, *z = 0*. Having just those two
dots and the fact, that voxel's faces are parallel to planes given by equations
above, we can define a voxel. Because of `vox_voxel` is a global variable, all
voxels in the library are of the same size.

**NB:** If you compile your library with `SSE_INTRIN` option, `vox_dot` will be
array of 4 single float values with the last of them being unused. You can
still, however, use the first three elements as usual.


### Creating a tree from an array of voxels
The first thing you must know how to do before doing anything else with these
libraries is creation of voxel trees. There are two ways of doing this. The
first way is creating from an array of voxels. Suppose you create an array `array`
which contains `n` voxels and make a new tree. Then you can do it as follows:

~~~~~~~~~~~~~~~~~~~~{.c}
vox_dot *array = malloc (n * sizeof(vox_dot));
fill_array (array, n); // Fill it as you wish
struct vox_node *tree = vox_make_tree (array, n);
free (array);
~~~~~~~~~~~~~~~~~~~~

`vox_make_tree()` creates a new voxel tree and `free()` destroys the array as it is
probably of no use for you anymore. Note, that if you wish to continue to use
your array, you must take into account that it is sorted in `vox_make_tree()`,
so any old indices to that array will no longer point to the same
elements. There are no functions in these libraries which require the old array
as an argument.

**NB:** All elements of the array must be unique and be multiple of
`vox_voxel`. If you have `vox_voxel`, say, {1,1,1}, then array element of value
{0.5, 0.7, 1.1} is invalid. Only whole numbers are valid here. This is probably
due to the library's misdesign, that it's strongly recommended to use values for
`vox_voxel` which fit good in single float precision. The default for
`vox_voxel` is {1,1,1}.

**NB:** If you compile your library with `SSE_INTRIN` option, arrays of
`vox_dot`s must be 16-byte aligned. You can use `aligned_alloc()` function from
standard C library for this.

### Manipulating and destroying the tree
You can get a number of voxels in the tree by calling `vox_voxels_in_tree()` or
get a bounding box for the tree with `vox_bounding_box()`. See API documentation
for details.

You can also insert and delete voxels in the existing tree. This is slower than
creating tree from an array. Not to be specific, I just say, that
`vox_make_tree()` can create trees of tens of millions voxels in one or two seconds,
while inserting in a live tree can give you rate just one million voxels per
second on the same machine. Let's see some code examples to learn how to do
this:

~~~~~~~~~~~~~~~~~~~~{.c}
vox_dot dot; 
int res;
struct vox_node *tree = NULL; // NULL is always understood as an empty tree
dot[0] = 1; dot[1] = 2; dot[2] = 3;
res = vox_insert_voxel (&tree, dot);
// Now res is 1 (success), and tree points to a newly created tree
res = vox_insert_voxel (&tree, dot);
// It's OK to try to insert the same voxel, res is now 0 (not successful)
res = vox_delete_voxel (&tree, dot);
// res is 1 (success), voxel is deleted and the tree has no voxels

// Assume now vox_voxel is {1,1,1} (default)
dot[0] = 0.2; dot[1] = 0.3; dot[2] = 1.5;
vox_insert_voxel (&tree, dot);
// This is OK, voxel with value {0,0,1} will be inserted.
// vox_insert_voxel() inserts vox_voxel-aligned voxel.
~~~~~~~~~~~~~~~~~~~~

I do not recommend to use these functions when you create a tree from scratch,
use `vox_make_tree()` instead. Many calls to `vox_insert_voxel()` or
`vox_delete_voxel()` can make your tree unbalanced. You can rebuild a tree
completely with `vox_rebuild_tree()` function.

There are 2 common patterns which insertion/deletion functions recognize.

* Inserting voxels one-by-one so there is no gap between them. This pattern is
  common in loops with counters like the one below.
  ~~~~~~~~~~~~~~~~~~~~{.c}
  for (k=0; k<K; k++)
  {
      for (l=0; l<L; l++)
      {
          for (m=0; m<M; m++)
          {
              dot[0] = vox_voxel[0]*k;
              dot[1] = vox_voxel[1]*l;
              dot[2] = vox_voxel[2]*m;
              vox_insert_voxel (&tree, dot);
          }
      }
  }
  ~~~~~~~~~~~~~~~~~~~~
  This will work very fast and will be very efficient in the sense of memory
  consumption, because of use of so-called "dense nodes" within the library.
  On the other hand, deletion from these dense nodes is very inefficient (about
  3 times slower than deletion from ordinary nodes).
  
* Insertion/deletion of the furthermost random voxels. When inserting/deleting
  random voxels, try to do it with furthermost voxles first. When inserting it
  will help to calculate internal data of voxel nodes properly, so voxels are
  distributed more evenly in the tree. When deleting it will also keep the tree
  balanced.

When the tree is no longer needed it must be destroyed with `vox_destroy_tree()`
function. Trees with no voxels in them need not to be destroyed (remember, they
are just `NULL`).

### Searching
This is the reason why **voxtrees** library exists. It can perform various types
of search much faster than if we would try to do it with exhaustive search.

You can check if a ball intersects any voxel in the tree with
`vox_tree_ball_collidep()`:
~~~~~~~~~~~~~~~~~~~~{.c}
vox_dot center = {0,0,0};
float radius = 10;
int interp = vox_tree_ball_collidep (tree, center, radius);
if (interp) printf ("There is/are voxel(s) close enough to {0,0,0}\n");
~~~~~~~~~~~~~~~~~~~~

You can find where a ray hits the first voxel on its path through the
tree. There is a function `vox_ray_tree_intersection()` for that.
~~~~~~~~~~~~~~~~~~~~{.c}
struct vox_node *leaf;
vox_dot origin = {0,0,0};
vox_dot direction = {1,1,1}; // May be a vector of any length
vox_dot intersection;
leaf = vox_ray_tree_intersection (tree, origin, direction, intersection);
if (leaf != NULL) printf ("There is intersection at <%f, %f, %f>\n",
                          intersection[0], intersection[1], intersection[2]);
~~~~~~~~~~~~~~~~~~~~
In the latter example the leaf node is returned where intersection is found or
`NULL` if there is no intersection. Note, that empty nodes (with no voxels in
them) are also `NULL`, but there are no intersections with them in any case.

Voxrnd
------
### Rendering
**voxrnd** is a rendering library which works in conjunction with **voxtrees**
library. The rendering is performed via a context object, `struct
vox_rnd_ctx`. You can choose one of two backends: a window managed by context or
a previously created SDL surface. A context which renders directly to window is
created by `vox_make_context_and_window()` and the one which renders to SDL
surface is created by `vox_make_context_from_surface()`. You can render then a
tree by calling `vox_render()` supplying the context as argument. Also you need
to attach your scene (currently it's just a tree) and a camera to your
context. This can be done by calling `vox_context_set_scene()` and
`vox_context_set_camera()`. If you render directly to window, you must call
`vox_redraw()` after `vox_render()` to redraw the window. `vox_render()` can be
called from any thread and `vox_redraw()` only from main (that one which called
`SDL_Init()`).
You can create the camera using constructor from a structure called camera
interface. **voxrnd** currently provides 2 camera classes, and therefore 2
implementations of camera interface. You can get an implementation of camera
interface by calling camera interface getter. One is `vox_simple_camera_iface()`
and the other is `vox_distorted_camera_iface()`. The first camera class features
a simple camera with 6 degrees of freedom, and the second is like the first, but
produces distorted projection like one produced by circular fish-eye camera.

To create a simple camera you must write something like this:
~~~~~~~~~~~~~~~~~~~~{.c}
struct vox_camera* camera = vox_simple_camera_iface()->construct_camera (NULL);
~~~~~~~~~~~~~~~~~~~~
`construct_camera()` here is a constructor. It's argument is another camera instance
or `NULL`. If it is not `NULL`, a newly created camera will inherit all internal
fields (as rotation angles, position, etc.) from supplied camera. The supplied camera
must be of the same class as the newly created or from a related class with the same
data layout. Behaviour is undefined if this condition does not hold. You can also set
the camera's field of view and position (otherwise they will remain at their default
values, depending on camera's implementation):
~~~~~~~~~~~~~~~~~~~~{.c}
camera->iface->set_fov (camera, 1.2);
vox_dot position = {-10, 10, 100};
camera->iface->set_position (camera, position); // Here argument is copied.

~~~~~~~~~~~~~~~~~~~~
Putting it all together you will get something
like this:

~~~~~~~~~~~~~~~~~~~~{.c}
struct vox_node *tree = vox_make_tree (voxels, n);
vox_dot origin = {0,0,0}; // Camera's origin
float fov = 1.2; // Camera's field of view
// Make a default camera
struct vox_camera *camera = vox_simple_camera_iface()->construct_camera (NULL);
camera->iface->set_position (camera, origin);
cmaera->iface->set_fov (camera, fov);
struct vox_rnd_ctx *ctx =
     vox_make_context_from_surface (surface);
vox_context_set_scene (ctx, tree);
vox_context_set_camera (ctx, camera);
vox_render (ctx);
SDL_SaveBMP (screen, "rendering.bmp");
camera->iface->destroy_camera (camera); // Destroy the camera
vox_destroy_tree (tree); // Destroy the tree
vox_destroy_context (ctx); // Destroy the context
SDL_FreeSurface (surface);
// And so on
~~~~~~~~~~~~~~~~~~~~
This will produce a visualisation of the tree saved in `rendering.bmp`

Another example is:
~~~~~~~~~~~~~~~~~~~~{.c}
struct vox_node *tree = vox_make_tree (voxels, n);
// Make a default camera as earlier
struct vox_camera *camera = vox_simple_camera_iface()->construct_camera (NULL);
// Make 800x600 window
struct vox_rnd_ctx *ctx =
     vox_make_context_and_window (800, 600);
vox_context_set_scene (ctx, tree);
vox_context_set_camera (ctx, camera);
// Render to internal surface
vox_render (ctx);
// And put the result on the screen
vox_redraw (ctx);
camera->iface->destroy_camera (camera); // Destroy the camera
vox_destroy_tree (tree); // Destroy the tree
vox_destroy_context (ctx); // Destroy the context
~~~~~~~~~~~~~~~~~~~~

### Cameras
Let's talk more about cameras and their interfaces. There are few structures to work
with cameras. The first is `struct vox_camera`. It is a generic camera class. All
interface functions accept objects of this class as their first
argument. Constructors also return objects of this type. There are more specified
camera classes as `struct vox_simple_camera` or `struct vox_distorted_camera`. There
is also `struct vox_camera_interface` structure. It contains a camera interface, in
other words, a set of functions, visible both to the library and to the user and
which any camera class must implement to define a working camera.

There are two types of methods in camera interface. The first is class methods, like
a camera constructor. The common pattern to call them is using camera interface
getter:
~~~~~~~~~~~~~~~~~~~~{.c}
camera_interface_getter()->class_method (args); // The pattern
vox_simple_camera_iface()->construct_camera (NULL); // An example
~~~~~~~~~~~~~~~~~~~~
The second is instance methods. The pattern for them is:
~~~~~~~~~~~~~~~~~~~~{.c}
vox_camera *camera;
// Initialization skipped
camera->iface->instance_method (camera, args); // The pattern
camera->iface->destroy_camera (camera); // An example
~~~~~~~~~~~~~~~~~~~~

As you can see, each camera object contains a copy of `struct vox_camera_interface`
structure where an implementation of camera interface is stored. To call an instance
method you must always use `camera->iface` to refer to the camera's methods. To call
a class method you can use both `camera->iface` (if you already have an instance of
that class) or camera interface getter.

The goal which is achieved by keeping camera interface and camera implementation
apart is to make it possible for user to define his/her own camera classes. You can
see how this can be done by seeing source code for 2 camera classes which are
available in **voxrnd** library. I'll try to make a guide for that later.
You can get more info on camera interface in `struct vox_camera_interface`
documentation.

### Collision detection
Unfortunately, currently you cannot move the whole tree to any direction, so there is no
tree-tree collision detection. But there is collision detection mechanism for camera. This
mechanism can be created by calling `vox_make_cd()` which returns an object of type
`struct vox_cd`. Later you attach your camera and your rendering context (that means, the
whole scene) to this structure. Once in a rendering loop you do `vox_cd_collide()`. This
automatically checks camera position and if it collides with any part of a tree, its
previous valid position is restored. Example:

~~~~~~~~~~~~~~~~~~~~{.c}
struct vox_rnd_ctx *ctx =
     vox_make_context_and_window (800, 600);
vox_context_set_scene (ctx, tree);
vox_context_set_camera (ctx, camera);
struct vox_cd *cd = vox_make_cd ();
vox_cd_attach_camera (cd, camera, 4); // 4 is the camera's body radius
vox_cd_attach_context (cd, ctx);
while (1) {
    vox_render (ctx);
    vox_cd_collide (cd);
    ....
}
done:
free (cd);
// And so on
~~~~~~~~~~~~~~~~~~~~

Voxengine
---------
**Voxengine** is somewhat a combine library, integrating all other libraries of
voxvision, SDL2 and lua scripting. A lifecycle of an application which uses
**voxengine** can be similar to the following:
    1. You create an engine by calling `vox_create_engine()`.
    2. You load your lua control script by calling `vox_engine_load_script()`.
    2. In infinite loop you call `vox_engine_tick()` to render a new frame and
       update the engine's state. Optionally you can do anything you want to do
       in this loop, e.g. use FPS counter.
    3. You check if lua control script has requested to stop the engine by
       calling `vox_engine_quit_requested()`.
    4. In case you deside to quit, jump out of the loop and call
       `vox_destroy_engine()`.
    5. Do all SDL event handling in lua script using **luasdl2** written by
       Tangent128 (see below). If you wish to quit, call `request_quit()`.

Here is an example:
~~~~~~~~~~~~~~~{.c}
int main (int argc, char *argv[])
{
    // Creating an engine and a window with dimensions 800x600
    struct vox_engine *engine = vox_create_engine (800, 600);
    if (engine == NULL) {
        // vox_create_engine reports an error.
        // Quit here
        return 1;
    }
    while (1) {
        vox_engine_tick (engine);
        if (vox_engine_quit_requested (engine)) goto done;
    }

done:
    vox_destroy_engine (engine);
    return 0;
}
~~~~~~~~~~~~~~~

There is an example program, **voxvision-engine**, in this project. It demonstates
the use of **voxengine**. You can see its source code in `src/demo2` directory.

The power of **voxengine** comes with lua. A lua control script (the one you passed
with `-s` argument, if you use **voxvision-engine**) is executed in protected
environment and must at least contain `init` function. This environment
includes (but is not limited to):
    * Standard `print`, `pairs`, `ipairs`, `next`, `unpack`, `tostring`, `tonumber`
      functions.
    * Almost all from math module in `math` table
    * `os.clock`
    * `table.insert`, `table.remove`, `table.maxn`, `table.sort` functions
    * `voxtrees`, `voxrnd` and `voxutils` modules
    * Some functions and tables from **luasdl2**, such as `getKeyboardState`,
      `pollEvent`, `pumpEvent`, `waitEvent`, `key`, `event`, `scancode` etc. in
      `voxsdl` table.

You can get the full list of available functions/tables/variables if you look at
`src/voxengine/genenvironment.py` and `src/voxengine/engine.c` files.

`init` function takes no arguments and must return a table. The table is your "world". It
must contain a tree to render and a camera to see through. Let's see an example:

~~~~~~~~~~~~~~~{.lua}
vt = voxtrees
vr = voxrnd
vs = voxsdl

function init ()
   -- Set voxel size
   vt.voxelsize {0.25, 0.25, 0.25}
   -- Create a set for up to 50*50*50 dots
   local a = vt.dotset(50*50*50)
   for i = 0,50 do
      for j = 0,50 do
         for k = 0,50 do
            -- Push a new dot to the set
            a:push {i,j,k}
         end
      end
   end
   -- Convert the set to a voxel tree
   local t = vt.settree (a)
   -- How many dots are in the tree?
   print (#t)

   -- Create a new simple camera
   local camera = vr.simple_camera()
   -- These methods are like methods in vox_camera_interface
   camera:set_position {25,-100,25}
   camera:look_at {25,25,25}
   camera:set_fov (0.45)
   -- You must return at least 2 values in table from init: a tree and a camera
   return {tree = t, camera = camera}
end

function tick (world, time)
   -- This is how events are handeled. Just like in C program
   local event, quit
   for event in vs.pollEvent() do
      if event.type == vs.event.KeyDown and event.keysym.sym == vs.key.Escape then
         quit = true
      elseif event.type == vs.event.Quit then
         quit = true
      end
   end
   -- Call request_quit when you wish to shut down the engine
   if quit then request_quit() end

   -- You can rotate camera or modify tree in tick() function
   local camera = world.camera
   time = time / 2000
   local dot = {25+125*math.sin(time),25-125*math.cos(time),25}
   camera:set_position (dot)
   camera:look_at {25,25,25}
end
~~~~~~~~~~~~~~~

Another function seen in this example is `tick`. It is called once in the main loop
when `vox_engine_tick()` is called and used to update the scene. It accepts 2
arguments: your world and time in milliseconds from the start of the
program. This function is optional. If you do not supply it, **voxengine** will
do basic SDL event handling by itself. Here is another example which brings more
interaction with user:

~~~~~~~~~~~~~~~{.lua}
vt = voxtrees
vr = voxrnd
vs = voxsdl

function init ()
   -- You can also create an empty tree
   local t = vt.tree()
   -- And add a voxel to it
   t:insert {0,0,0}

   local camera = vr.simple_camera()
   camera:set_position {0,-10,0}

   --[[
        Here is collision detector.
        Its interface is like its C equivalent, only it lacks attach_context() method.
        Context which contains returned tree is attached by the engine automatically
   ]]--
   local cd = vr.cd()
   cd:attach_camera (camera, 4)

   -- Collision detector is returned in 'cd' field
   return {tree = t, camera = camera, cd = cd}
end

function tick (world, time)
   local event, quit
   for event in vs.pollEvent() do
      if event.type == vs.event.KeyDown and event.keysym.sym == vs.key.Escape then
         quit = true
      elseif event.type == vs.event.Quit then
         quit = true
      end
   end
   if quit then request_quit() end

   -- Here you can get the keyboard state with 'getKeyboardState' function
   local keystate = vs.getKeyboardState()
   local camera = world.camera
   --[[
        You can check if a key is pressed in the following way.
        This example implements WASD-movement and camera rotation.
   ]]--
   if keystate[vs.scancode.S] then
      camera:move_camera {0,-5,0}
   elseif keystate[vs.scancode.W] then
      camera:move_camera {0,5,0}
   end

   if keystate[vs.scancode.A] then
      camera:move_camera {-5,0,0}
   elseif keystate[vs.scancode.D] then
      camera:move_camera {5,0,0}
   end

   if keystate[vs.scancode.Left] then
      camera:rotate_camera {0,0,0.05}
   elseif keystate[vs.scancode.Right] then
      camera:rotate_camera {0,0,-0.05}
   end
   
   if keystate[vs.scancode.Up] then
      camera:rotate_camera {-0.05,0,0}
   elseif keystate[vs.scancode.Down] then
      camera:rotate_camera {0.05,0,0}
   end
end
~~~~~~~~~~~~~~~

**Voxengine**'s lua interface can do primitive keyboard handling by itself. It can also
understand raw data files. Please look at lua scripts in `example` directory.

All memory required for such objects as trees, dotsets etc. is handeled by lua automatically.

Demo application
----------------
This application is meant as a demonstration of **voxvision** libraries and as a
developer's playground. I'll provide information on it here too.

### Controls
Here is default keys for basic controls:

| Action         | Key         |
| ---            | ----------- |
| Tilt left      | `Z`         |
| Tilt right     | `X`         |
| Walk left      | `A`         |
| Walk right     | `D`         |
| Walk forwards  | `W`         |
| Walk backwards | `S`         |
| Fly up         | `1`         |
| Fly down       | `2`         |
| Insert cube    | `I`         |
| Delete cube    | `O`         |

### Scene configuration files
Configuration files are just old Windows `ini` files inside. There are 3 types
of values there: numbers, vectors and strings. Numbers are any numbers: integers
or floats, does not matter. A vector is triplet or (maybe) pair of numbers in
format `<x,y,z>` or `<x,y>`, i.e. enclosed in angle brackets and separated by
commas. Strings are enclosed in double quotes (`"`). Here is a table for each
possible key and its meaning in scene configuration file:

| Section:Key        | Type     | Defaults to  | Comment                          |
| -------------      | ------   | -----------  | --------                         |
| `Scene:DataSet`    | string   | N/A          | Dataset's file name, mandatory   |
| `Scene:Voxsize`    | vector   | `<1,1,1>`    | Size of a voxel                  |
| `Scene:Geometry`   | vector   | N/A          | Dimensions of dataset, mandatory |
| `Scene:Threshold`  | number | `30` | Samples with value bigger than that are loaded |
| `Scene:SampleSize` | number   | `1`          | Size of sample (in bytes)        |
| `Camera:Position`  | vector   | `<0,-100,0>` | Start with that camera position  |
| `Camera:Fov`       | number   | `1`          | Camera's field of view           |
| `Camera:Rot`       | vector   | `<0,0,0>`    | Start with that camera rotation  |

Dataset is just a 3d array of little-endian binary coded samples, beginning with
the first sample, immediately followed by another and so on, all having one
sample size.

### Global configuration file
Here you can remap controls and set window properties. Look at the table:

| Section:Key            | Type     | Defaults to  | Comment         |
| --------------------   | ------   | -----------  | --------        |
| `Window:Width`         | number   | `800`        | Window's width  |
| `Window:Height`        | number   | `600`        | Window's height |
| `Controls:MouseXSpeed` | number   | `0.01`       | Mouse horizontal speed (May be less than zero to invert axis) |
| `Controls:MouseYSpeed` | number   | `0.01`       | Mouse vertical speed (May be less than zero to invert axis) |
