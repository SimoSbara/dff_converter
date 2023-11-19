# dff_converter

## Description
[RenderWare DFF+TXD](https://gtamods.com/wiki/RenderWare_binary_stream_file) converter to multiple standard 3D formats.
During the conversion it will preserve UV coordinates, normals and multiple materials and texture images.

It can convert to:
* [GLTF](https://www.khronos.org/gltf/) without .bin file;
* more coming...

Libraries used:
* [LodePNG](https://github.com/lvandeve/lodepng) for png encoding/decoding;
* [RWTools](https://github.com/aap/rwtools) for renderware DFF and TXD encoding/decoding;
* [TinyGLTF](https://github.com/syoyo/tinygltf) for writing/reading a GLTF model.

## Before build
Pull tinygltf external: `git submodule init && git submodule update`

## Build on Linux or MacOS

* Create a build folder and go in it: `mkdir build && cd build`
* Run CMake: `cmake ..`
* Run Make: `make install`

## Build on Windows
I generally use Visual Studio 2019+ for CMake.
Just open the main folder on VS and make it run, select Debug or Release config and compile or install. That's it!

## Requirements for Devs
* CMake 3.0.0+
* C++17

## Todo list
* GLTF bones conversion
* FBX conversion
* OBJ+MTL conversion

