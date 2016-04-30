Voxvision How To        {#mainpage}
===============

Voxtrees
--------

### Defining a voxel
Each voxel in the **voxtrees** library is represented by the type `vox_dot`,
which is just an array of 3 single float numbers. This dot type declares a
vertex of a voxel with the minimal coordinates. If you add `vox_voxel` global
variable to this dot, you will get another vertex of your voxel with maximal
coordinates. You can imagine a Cartesian coordinate system with origin {0,0,0}
and axes given by equations *x = 0*, *y = 0*, *z = 0*. Having just those two
dots and the fact, that voxel's faces are parallel to planes given by equations
above, we can define a voxel. Because of `vox_voxel` is a global variable, all
voxels in the library are of the same size.

**NB:** If you compile your library with `SSE_INTRIN` option, `vox_dot` will be
array of 4 single float values with the last of them being unused. You can
still, however, use the first three elements as usual.


### Creating a tree from an array of voxels
The first thing you must know how to do before doing anything else with these
libraries is creating of voxel trees. There are two ways of doing this. The
first way is creating from an array of voxels. Suppose you create an array `array`
which contatins `n` voxels and make a new tree. Then you can do it as follows:

~~~~~~~~~~~~~~~~~~~~{.c}
vox_dot *array = malloc (n * sizeof(vox_dot));
fill_array (array, n); // Fill it as you wish
struct vox_node *tree = vox_make_tree (array, n);
free (array);
~~~~~~~~~~~~~~~~~~~~

`vox_make_tree()` creates a new voxel tree and also destroys the array as it is
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
`vox_dot`s must be 16-byte aligned. You can use `aligned_alloc()` function for
this.

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
use vox_make_tree() instead. Many calls to vox_insert_voxel() or
vox_delete_voxel() can make your tree unbalanced. You can rebuild a tree
completely with `vox_rbuild_tree()` function.

There are 2 common patterns which insertion/deletion functions recognise.

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
This is the reason why `voxtrees` library exists. It can perform various types
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
NULL if there is no intersection. Note, that empty nodes (with no voxels in
them) are also NULL, but there are no intersections with them in any case.

Voxrnd
------
**voxrnd** is a rendering library which works in conjunction with **voxtrees**
library. You can render a tree to a `SDL_Surface` with `vox_render()`
function. To do this you must first create a renderer context with function
`vox_make_renderer_context()`. It accepts three arguments: the surface, the
scene (your tree) and the camera interface. You can get the interface by
creating a simple camera with `vox_make_simple_camera()` function. The goal is
to separate camera implementation (which can be redefined by user in his/her own
camera class) and camera methods (interface) which is more or less standard and
which is used in the library. Putting it all together you will get something
like this:

~~~~~~~~~~~~~~~~~~~~{.c}
struct vox_node *tree = vox_make_tree (voxels, n);
SDL_Surface *screen = SDL_SetVideoMode(800, 600, 32, SDL_SWSURFACE);
vox_dot origin = {0,0,0}; // Camera's origin
float fov = 1.2; // Camera's field of view
// Make a default camera
vox_simple_camera *camera = vox_make_simple_camera (fov, origin);
struct vox_rnd_ctx *ctx =
     vox_make_renderer_context (surface, tree, camera->iface);
vox_render (ctx);
SDL_SaveBMP (screen, "rendering.bmp");
free (ctx); // Free context after use
camera->iface->destroy_camera (camera); // Destroy the camera
vox_destroy_tree (tree); // Destroy the tree
// And so on
~~~~~~~~~~~~~~~~~~~~

This will produce a visualisation of the tree. You can get more info on camera
methods in `struct vox_camera_interface` documentation. The current "simple
camera" implementation supports collision detection and 6 degrees of freedom
(full translation and rotation in 3-dimensional space). The most common pattern
to call these methods is
~~~~~~~~~~~~~~~~~~~~{.c}
vox_simple_camera *camera;
// Initialisation skipped
camera->iface->method_name (camera, arg1, arg2, ...);
~~~~~~~~~~~~~~~~~~~~
So the first argument to any method is a camera object itself, followed by other
arguments. Currently, if you want to implement your own camera class, you must
re-implement all of those methods by yourself. It's possible, but very
unpractical, so the whole design is a subject to refactoring.
