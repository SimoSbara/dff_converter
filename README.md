# dff_converter

## Description
[RenderWare DFF+TXD](https://gtamods.com/wiki/RenderWare_binary_stream_file) converter to multiple standard 3D formats.
During the conversion it will preserve UV coordinates, normals and multiple materials and texture images.

It can convert to:
* [GLTF](https://www.khronos.org/gltf/) without .bin file;
* more coming...

Libraries used:
* [CxImage](https://www.codeproject.com/Articles/1300/CxImage) for multiple format image encoding/decoding;
* [RWTools](https://github.com/aap/rwtools) for renderware DFF and TXD encoding/decoding;
* [TinyGLTF](https://github.com/syoyo/tinygltf) for writing/reading a GLTF model.

## Requirements for Devs
* Visual Studio 2022+
* C++17

## Todo list
* GLTF bones conversion
* FBX conversion
* OBJ+MTL conversion

