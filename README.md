# Vesper
Render Engine Using the Vulkan API

This engine was created from scratch to explore the Vulkan API and learn more about this modern graphics API. I am not a rendering or engine programmer, so please bear that in mind!

> "Vesper" means evening in Latin

The main and important distinction between this engine and many others is that it is entirely powered by my own [ECS](https://github.com/KabalMcBlade/ECS-API) library. 
When I say "fully," I mean every part of it: objects to be rendered must have a vertex and/or an index buffer component assigned, just to name one example


## Installation Notes

### Inclusion and relative paths

This project uses vma and glm (for now). Both of them have been referenced from VulkanSDK, so for a path point of view, you should have:<br />
C:\VulkanSDK\{version}\Include\glm<br />
C:\VulkanSDK\{version}\Include\vma<br />
which in my case {version} is 1.3.261.1<br />
<br />
This is not mandatory but avoid you to add or change various paths in the project files.

### Linking

Currently the project is set as dynamic library DLL, but can be built as static library LIB if you'd like.<br />
The different settings for linkage are, in Windows and Visual Studio 2022:<br />
<br />
- Dynamic Library:
  - Vesper Engine
    - Runtime Library: Multi-threaded DLL (/MD) **OR** Multi-threaded Debug DLL (/MDd) 
    - Preprocessor macro to add: ECS_DLL_EXPORT;VESPERENGINE_DLL_EXPORT;
  - Viewer or any other executable
    - Runtime Library: Multi-threaded DLL (/MD) **OR** Multi-threaded Debug DLL (/MDd) 
    - Preprocessor macro to add: ECS_DLL_IMPORT;VESPERENGINE_DLL_IMPORT;
- Static Library:
  - Vesper Engine
    - Runtime Library: Multi-threaded (/MT) **OR** Multi-threaded Debug (/MTd)
  - Viewer or Any executable
    - Runtime Library: Multi-threaded (/MT) **OR** Multi-threaded Debug (/MTd)

### Features

- ECS core
- Latest Vulkan API
- Automatically support bindless and standard binding resource (textures and buffers)
- Opaque pipeline render system (for now just this)
- Push constant and specialization constant support integrated
- Extendable render pipeline

## Screenshots

I will post screenshots here as new features are added and when there’s something significant to show.


First render from the engine.
<img src="./Screenshots/1.png">

Second render from the engine, OBJ loaded using Phong shader rendering.
<img src="./Screenshots/2.png">


## LICENSE

- **Vesper Engine and Viewer**: MIT

- Assets (if not mine):
	- **A_blonde_twintailed_g_1228205950_texture** is downloadable from FAB store (https://www.fab.com/listings/e8be1218-232d-45a7-94cc-71ad83750803) under Creative Commons Attribution (CC BY 4.0): https://creativecommons.org/licenses/by/4.0/
 	- **colored_cube**, **cube**, **flat_vase**, **quad**, **smooth_vase** they have all been found around on different tutorials and pages, without any explicit license. They are pretty generic, so I guess they are Creative Commons Attribution.
 	- **Yokohama3** cubemap is the work of Emil Persson, aka Humus, and it is under Creative Commons Attribution (CC BY 3.0) Unported License: https://creativecommons.org/licenses/by/3.0/

- Shaders (if not mine):
	- **brdf_lut_shader.frag** source code is Copyright (c) 2018-2023, Sascha Willems - under MIT

# Build Status

| Platform | Build Status |
|:--------:|:------------:|
| Windows (Visual Studio 2022) | [![MSBuild](https://github.com/KabalMcBlade/Vesper/actions/workflows/msbuild.yml/badge.svg)](https://github.com/KabalMcBlade/Vesper/actions/workflows/msbuild.yml) |


