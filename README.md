# Vesper
Render engine using Vulkan API

This is made from scratch following in order to learn more about Vulkan API, just to learn something about this new graphic API. 
I'm not a render or engine programmer, so be aware!

> Vesper means Evening in Latin

The important and main difference across usual engine out there, is that is fully powered by my own [ECS](https://github.com/KabalMcBlade/ECS-API) library.
When I say "fully" I mean it, also the objects, to be rendered, have to have assigned a vertex and /or an index buffer component, just to name one.

## Screenshots

Here I will post the screenshots while something new will be added and worth to show.


First render of the engine, using ECS core.
<img src="./Screenshots/first.jpg">


## Demo Controls

For now the demo just load a couple of cubes and 2 obj vase, adding a point light.
Will evolve in a proper viewer in the future.


## LICENSE

GPL-3.0


# Build Status

| Platform | Build Status |
|:--------:|:------------:|
| Windows (Visual Studio 2022) | [![Build status](https://ci.appveyor.com/api/projects/status/30qjfjlc7fodhceb?svg=true)](https://ci.appveyor.com/project/KabalMcBlade/Vesper) |
