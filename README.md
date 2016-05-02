Voxvision
=========

What is Voxvision?
-----------------
**Voxvision** is a library (two of them to be accurate) for creating and
operating on voxel octrees and also for visualising sets of voxels. It also
contains a demo application which shows functionality of these libraries.

What is a voxel?
----------------
A voxel is a small cube in space. Its coordinates are given in a Cartesian
coordinate system and its faces are parallel to axes of that coordinate
system. This libary limitation is that all voxels have the same size.

What can it do?
---------------
The first library, *voxtrees*, is not very feature-rich for now. It can
construct voxel octrees and perform two types of search in octrees, which I
think are of the great importance. The first one checks if an arbitrary ray
hits any voxel in the tree. The second one checks if an arbitrary ball
intersects with any voxel. These checks are performed much faster than a naïve
O(n) voxel-by-voxel search.

The second library, *voxrnd*, is a SDL-based renderer of voxel datasets with 6
degrees of freedom, collision detection and multicore parallel processing.

Is there any API documentation?
-------------------------------
You can try to execute `make doc` from building directory (see below) to
generate an API documentation. But API changes too fast and unpredictable, so no
guarantees. Also this documentation contains a basic how-to for these libraries.

What are understandable formats of datasets used in the library?
----------------------------------------------------------------
There is no special format the library can work with for now, it understands
only arrays of voxels. The demo application provides an example of how a dataset
can be loaded in memory. All datasets are simple raw binary files. For example,
you can visit http://www.volvis.org/ to get some of datasets. Also you need to
write a simple configuration file to work with dataset. See `.cfg` files in
`example` directory for info.

Demo application
----------------
Usage: `voxvision-demo [-c global.cfg] dataset.cfg` where dataset.cfg is a
configuration file for dataset. TODO: describe global.cfg and control.

Requirements
------------
* SDL2
* [iniparser](https://github.com/ndevilla/iniparser) for demo application,
  optional
* CUnit for unit tests, optional
* [GCD](https://en.wikipedia.org/wiki/Grand_Central_Dispatch) for parallel
  rendering, optional, but highly recommended.
* Doxygen for API documentation, optional

Building
--------
From directory containing this file:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make
make install
```
Last step is optional. Also you can add `-DSSE_INTRIN=ON` to the third line to
enable some SSE intrinsics. It will speed up tree creation considerably. It's
better to use clang to build the library.

*TODO: tell about internals, API, configuration files*
