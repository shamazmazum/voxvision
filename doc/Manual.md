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
![A voxel in voxtrees library](voxel.png)

**NB:** If you compile your library with `SSE_INTRIN` option, `vox_dot` will be
array of 4 single float values with the last of them being unused. You can
still, however, use the first three elements as usual.

**NB:** It is not recommended to set values to objects of `vox_dot` type using
  indices, like `dot[i]`. Insted use `vox_dot_set()` macro.


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
vox_dot_set (dot, 1, 2, 3); // Like dot[0] = 1; dot[1] = 2; dot[2] = 3;
res = vox_insert_voxel (&tree, dot);
// Now res is 1 (success), and tree points to a newly created tree
res = vox_insert_voxel (&tree, dot);
// It's OK to try to insert the same voxel, res is now 0 (not successful)
res = vox_delete_voxel (&tree, dot);
// res is 1 (success), voxel is deleted and the tree has no voxels

// Assume now vox_voxel is {1,1,1} (default)
vox_dot_set (dot, 0.2, 0.3, 1.5);
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
              vox_dot_set (dot,
                           vox_voxel[0]*k,
                           vox_voxel[1]*l,
                           vox_voxel[2]*m);
              vox_insert_voxel (&tree, dot);
          }
      }
  }
  ~~~~~~~~~~~~~~~~~~~~
  This will work very fast and will be very efficient in the sense of memory
  consumption, because of use of so-called "dense nodes" within the library.
  On the other hand, deletion from these dense nodes is very inefficient (about
  3 times slower than deletion from ordinary nodes). Also please note, that if
  all you deserve is a tree consisting of just one big box, you can just use
  `vox_make_dense_leaf()` function instead of inserting voxels one-by-one in a
  loop.
  
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
of search much faster than if we would try to do it with naïve `O(n)` search.

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
surface is created by `vox_make_context_from_surface()`. Then you can render a
tree by calling `vox_render()` supplying the context as argument. Also you need
to attach your scene (currently it's just a tree) and a camera to your
context. This can be done by calling `vox_context_set_scene()` and
`vox_context_set_camera()`. After you call `vox_render()`, you must call
`vox_redraw()` to copy rendered image from the context's internals to the screen
or surface. The reason for why `vox_render()` and `vox_redraw()` are separated
is because `vox_render()` can be called from any thread and `vox_redraw()` only
from the main one (that one which called `SDL_Init()`).

You can create the camera using constructor from a structure called camera
interface. **voxrnd** currently provides 2 camera classes, and therefore 2
implementations of camera interface. You can get an implementation of camera
interface by calling camera methods getter, `vox_camera_methods()`. It accepts a
camera name as its only argument. Possible camera names are: `"simple-camera"`
and `"doom-camera"`. The first camera, despite the name, is fully functional
camera with six degrees of freedom, and the second is an old-fashioned but fast
doom-like camera with five degrees of freedom.

To create a simple camera you must write something like this:
~~~~~~~~~~~~~~~~~~~~{.c}
struct vox_camera* camera = vox_camera_methods ("simple-camera")->construct_camera (NULL);
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
struct vox_camera *camera = vox_camera_methods ("simple-camera")->construct_camera (NULL);
camera->iface->set_position (camera, origin);
camera->iface->set_fov (camera, fov);
struct vox_rnd_ctx *ctx =
     vox_make_context_from_surface (surface);
vox_context_set_scene (ctx, tree);
vox_context_set_camera (ctx, camera);
vox_render (ctx);
vox_redraw (ctx);
SDL_SaveBMP (surface, "rendering.bmp");
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
struct vox_camera *camera = vox_camera_methods ("simple-camera")->construct_camera (NULL);
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
### Quality settings
![Rendering pass in voxrnd](rnd.png)
**voxrnd** performes an important optimization which allows it to work more or
less quickly, but has a penalty in quality degradation, so this is a
quality-speed trade-off. To controll the balance, there are three quality modes
in **voxrnd**. To understand it, you need to understand how **voxrnd**
works. First of all, for any context it allocates an array of blocks 4x4 pixels
each. Here comes current restriction on screen resolution: its width must be an
integral multiple of 16, and its height must be an integral multiple of 4.
Fortunately, all mainstream resolutions, like 640x480, 800x600, 1280x1024 or
1920x1080 are OK. Then, when you call `vox_render()`, **voxrnd** casts a ray for
each pixel on the screen (currently, exactly one ray for one pixel). The process
is sequential for each pixel in a block, but is parallelized between the blocks
(see picture above, where solid black squares are blocks, red squares are
pixels and Z-shaped line is an order which is used to render a
block). Parallelization means that you can benifit from multicore CPU. As you
can see, rendering inside a block is performed in Z-order. When our rendering is
ready, `vox_redraw()` copies array of block into an ordinary SDL surface which
is rendered to the screen.

There is our optimization: for any next pixel **voxrnd** does not run a search
in the tree starting from its root node, but it uses a leaf node obtained from
the previous search. It runs from the root only if this mechanism does not find
an intersection. Surely, this adds some rendering artifacts (usually, they are
observed at edges of objects), but speeds up things a lot. The quality setting
for this mode is called *fast*.

There is also the setting called *best*. In this mode, the search is started
from the root for each pixel.

Finally, there is *adaptive* mode. In this mode, the renderer chooses from
*fast* and *best* mode for each block individually. The choice is depending on
distance between intersections for the first and the last pixels in the block
(see picture). This gives a rendering similar to one with *best* quality and
rendering speed somewhat in between *best* and *fast* (it depends on the tree to
be rendered).

All these modes are selected using `vox_context_set_quality()` function which
you can call anytime between two `vox_render()` calls. The defalut mode is
*adaptive*.

### Cameras
Let's talk more about cameras and their interfaces. There are few structures to work
with cameras. The first is `struct vox_camera`. It is a generic camera class. All
interface functions accept objects of this class as their first
argument. Constructors also return objects of this type. There are more specified
camera classes, which are implemented as shared modules (plug-ins). As mentioned
before, standard modules are `simple-camera` and `doom-camera`. There is also
`struct vox_camera_interface` structure. It contains a camera interface
implementation, in other words, a set of functions, visible both to the library
and to the user and which any camera class must implement to define a working
camera. Of course, often you may wish to implement two or more camera classes,
which share their data layout (defined by the same structure), but implement
camera interface differently (like FPS camera and third-person view
camera). Then you can "inherit" some of the basic methods (like setting camera's
position or field of view) using `vox_use_camera_methods()` function, which
takes `struct vox_camera_interface` structure and copies all defined methods
from it to a specified camera.

There are two types of methods in camera interface. The first is class methods, like
a camera constructor. The common pattern to call them is using camera methods
getter, `vox_camera_methods()`:
~~~~~~~~~~~~~~~~~~~~{.c}
vox_camera_methods (cam_name)->class_method (args); // The pattern
vox_camera_methods ("doom-camera")->construct_camera (NULL); // An example
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
that class) or camera methods getter.

The goal which is achieved by keeping camera interface and camera implementation
apart is to make it possible for user to define his/her own camera classes. You can
see how this can be done by seeing source code for 2 camera classes which are
available in **voxrnd** library. I'll try to make a guide for that later.
You can get more info on camera interface in `struct vox_camera_interface`
documentation.

![A coordinate system of a camera](camcoord.png)
On the picture above you can see the camera's coordinate system. It is projected
on the screen exactly as on the picture (i.e. the camera's X maps exactly to the
screen's right, etc). The Y direction is always "forwards" for you. As for
rotation around the axes, the positive direction is clockwise. Be careful with
coordinate systems, as some of the camera's methods require coordinates in its
own coordinate system (these are usually "incremental" methods as
`camera->iface->move_camera()`) and some require coordinates in the world's
coordinate system (these usually do some "absolute" or "non incremental" stuff,
an example: `camera->iface->set_position()`).

### Collision detection
Unfortunately, currently you cannot move the whole tree to any direction, so there is no
tree-to-tree collision detection. But there is collision detection mechanism for
a camera. This mechanism can be created by calling `vox_make_cd()` which returns
an object of type `struct vox_cd`. Later you attach your camera and your
rendering context (that means, the whole scene) to this structure. Once in a
rendering loop you do `vox_cd_collide()`. This automatically checks camera
position and if it collides with any part of a tree, its previous valid position
is restored. An example:

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

### FPS control
**voxrnd** has its own FPS counter and controller. It is created using
`vox_make_fps_controller()` function and destroyed with
`vox_destroy_fps_controller()`. `vox_make_fps_controller()` takes a desired
number of frames per second (or just `0` if you do not want to restrict value of
FPS and want just to count it). It returns a block of type
`vox_fps_controller_t` which you must call once in a rendering loop to do its
work. Look at this for example:

~~~~~~~~~~~~~~~~~~~~{.c}
// Restrict the value of frames per second by 30
vox_fps_controller_t fps_counter = vox_make_fps_controller (30);
while (keep_rendering) {
    .......
    vox_render (ctx);
    vox_redraw (ctx);
    .......
    // fps_info contains some useful info about actual FPS rate and frame
    // rendering time.
    struct vox_fps_info fps_info = fps_counter();
}
done:
vox_destroy_fps_controller (fps_counter);
~~~~~~~~~~~~~~~~~~~~
See API documentation for more info.

Voxengine
---------
**Voxengine** is somewhat a combine library, integrating all other libraries of
voxvision, SDL2 and lua scripting. A lifecycle of an application which uses
**voxengine** can be similar to the following:
    1. You create an engine by calling `vox_create_engine()` passing your lua
        control script as a parameter.
    2. In infinite loop you call `vox_engine_tick()` to render a new frame and
       update the engine's state. Optionally you can do anything you want to do
       in this loop, e.g. use FPS counter.
    3. You check if lua control script has requested to stop the engine by
       calling `vox_engine_quit_requested()` passing the status returned by
       `vox_engine_tick()` as a parameter.
    4. In case you deside to quit, jump out of the loop and call
       `vox_destroy_engine()`.
    5. Do all SDL event handling in lua script using **luasdl2** written by
       Tangent128 (see below). If you wish to quit, return `false` from `tick()`
       function, otherwise return `true` (see below).

Here is an example:
~~~~~~~~~~~~~~~{.c}
int main (int argc, char *argv[])
{
    /*
     * Creating an engine and a window with dimensions 800x600. The last two
     * arguments can be ignored now (they are explained in API documentation.
     */
    struct vox_engine *engine = vox_create_engine (800, 600, script, 0, NULL);
    if (engine == NULL) {
        // vox_create_engine reports an error.
        // Quit here
        return 1;
    }
    while (1) {
        vox_engine_status status = vox_engine_tick (engine);
        if (vox_engine_quit_requested (status)) goto done;
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
   local camera = vr.camera "simple-camera"
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
   -- Return nil when you wish to shut down the engine
   if quit then return nil end

   -- You can rotate camera or modify tree in tick() function
   local camera = world.camera
   time = time / 2000
   local dot = {25+125*math.sin(time),25-125*math.cos(time),25}
   camera:set_position (dot)
   camera:look_at {25,25,25}

   return true
end
~~~~~~~~~~~~~~~

Another function seen in this example is `tick`. It is called once in the main loop
when `vox_engine_tick()` is called and used to update the scene. It accepts 2
arguments: your world and time in milliseconds from the start of the
program. It must always return `true` unless you want to quit. This function is
optional. If you do not supply it, **voxengine** will do basic SDL event
handling by itself. Here is another example which brings more interaction with
user:

~~~~~~~~~~~~~~~{.lua}
vt = voxtrees
vr = voxrnd
vs = voxsdl

function init ()
   -- You can also create an empty tree
   local t = vt.tree()
   -- And add a voxel to it
   t:insert {0,0,0}

   local camera = vr.camera "simple-camera"
   camera:set_position {0,-10,0}

   --[[
        Here is a collision detector.
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
   if quit then return nil end

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

   return true
end
~~~~~~~~~~~~~~~

**Voxengine**'s lua interface can interact with SDL by means of
[**luasdl2**](https://github.com/Tangent128/luasdl2). It can also understand raw
data files, using `vox_read_raw_data()` from **voxtrees**. Please look at lua
scripts in `example` directory. All memory required for such objects as trees,
dotsets etc. is handeled by lua automatically.

**NB:** If you wish to modify your tree in `tick()` function in any way, please
  wrap the tree in `voxrnd.scene_proxy()` in init function like so:
  ~~~~~~~~~~~~~~~{.lua}
  function init ()
      local tree = voxtrees.tree()
      .....
      return {tree = voxrnd.scene_proxy (tree), ...} -- Not just "tree = tree"
  end
  ~~~~~~~~~~~~~~~
  Then in `tick()` function you may alter the tree by `world.tree:insert()` or
  `world.tree:delete()` as usual. `voxrnd.scene_proxy()` provides all necessary
  synchronization between the renderer and insertion/deletion/rebuilding
  operations.

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
