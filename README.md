Voxvision
=========
![CI](https://github.com/shamazmazum/voxvision/workflows/CI/badge.svg)

What is Voxvision?
-----------------
**Voxvision** is a library (few of them to be accurate) for creating and
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

And the third library, *voxengine* manages the first two, opening windows,
loading data etc. and provides lua scripting with which you can describe your
scene, control camera, do simple keyboard handling etc.

Is there any API documentation?
-------------------------------
You can try to execute `make doc` from building directory (see below) to
generate an API documentation. But API changes too fast and unpredictable, so no
guarantees. Also this documentation contains a basic tutorial for these
libraries.

If you do not want to install doxygen, you can just visit
[voxvision](http://shamazmazum.github.io/voxvision) site on Github Pages.

What are understandable formats of datasets used in the library?
----------------------------------------------------------------
There is no special format the library can work with for now, but *voxtrees* can
handle simple raw binary format (see documentation and examples). For example,
you can visit http://www.volvis.org/ to get some of datasets. Also you need to
write a simple configuration file to work with dataset (or, if you use
*voxengine*, write a loading script). Few datasets and configuration (`.cfg`)
files present in `example` directory.

Demo application
----------------
Usage: `voxvision-demo [-c <global_config>] <dataset_config>` where
`dataset_config` is a configuration file for dataset and `global_config` is an
optional global configuration file. These files are in `ini` format (as in
Microsoft Windows). See tutorial for more info.

Also, with version 0.20, here comes *voxengine* library and simple program,
which uses its capabilities.
Usage: `voxvision-engine [-w width] [-h height] [-f fps] -s script`. `script` is
lua control script, look at `examples` directory.

Requirements
------------
* SDL2
* [iniparser](https://github.com/ndevilla/iniparser) for demo application,
  optional
* CUnit for unit tests
* [GCD](https://en.wikipedia.org/wiki/Grand_Central_Dispatch) for parallel
  rendering, optional, but highly recommended.
* Doxygen for API documentation, optional
* Lua >= 5.2, optional, but highly recommended for *voxengine* library.
* Clang or other compiller with support for blocks. GCC will not do.

Building
--------
From directory containing this file:
```
git submodule init
git submodule update
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RELEASE ..
make
make install
```
Last step is optional. Also you can add `-DSSE_INTRIN=OFF` to the third line if
you have old hardware. If you do not have GCD, add `-DWITH_GCD=OFF`.

For more info visit [the project page](http://shamazmazum.github.io/voxvision)

Special thanks
--------------

I want to thank Tangent128 for his luasdl2, lua binding to SDL2 which I currently
use.
