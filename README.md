# Elever Render
## A multi-platform vendor independent photorealistic rendering engine

![alt text](https://i.imgur.com/G7ExwxZ.png)
*Scene rendered with Eleven. 1000 samples and denoised*

Eleven Render is my personal project. It's a unidirectional C++ path tracing rendering engine, where compatibility prevails over other aspects. Thanks to SYCL, it's possible to compile it for almost any GPU/CPU.
It also seeks for broad compatibility with 3D Model software thanks to ASL, a shading language I'm working on, which allows other shading languages (like OSL), to be converted to it.

At the moment, Eleven is a WIP, and features like ASL are just proof of concepts. 


## Releases
The only way to use Eleven Render right now is through Blender, thanks to the [ElevenBlender plug-in](https://github.com/101001000/ElevenBlender) 
Download the last ElevenBlender release here:


## Features

- Compatibility with both GPU and CPU (vendor independent)
- Normal recalculation with Vertex Weighted algorithm (so if your model has broken normals you won't have to worry about)
- Osl compatibility through a custom shading language
- MIS (BRDF, Environment, PointLights)
- Denoising (Open Image Denoiser) 

## Build
### Dependencies
- [SYCL]() 
- [Boost](https://github.com/boostorg/boost) for TCP, logging, program args, 
- [Intel Open Image Denoiser](https://github.com/OpenImageDenoise/oidn) for final denoise
- [Flex](https://github.com/westes/flex) for ASL parsing
- [Bison](https://github.com/akimd/bison) for ASL parsing
- [Mikktspace](https://github.com/mmikk/MikkTSpace) for generating the tangent space
- [Rapidobj](https://github.com/guybrush77/rapidobj) for loading wavefront objects
- [stb_image & stb_image_write](https://github.com/nothings/stb) for image IO

## Docs

<details open>
<summary>Gallery</summary>
<br>
</details>

## Disclaimer

Eleven Render is the result of my Bachelor's Thesis and now my Master's Thesis. The way I worked on it was, to get the most features, in the smallest possible time, so most of the code is messy and unorganized. I'm working on modernizing the code and cleaning it up, as well as documenting it. Most of features are incomplete. The shading is giving me issues, so the implementation is probably not correct. The efficiency is also not the best and it's full of memory leaks. I hope to have all this sorted out in the following months, but for now, this is the render's state.

## ASL
Eleven Render runs a custom formally specified shading language called Abstract Shading Language. ASL tries to unify shading languages, acting as middle step between shading code and Eleven. The current implementation, just covers an extremely inefficient intepretation of such a language. Compilation is planned in the future, but the current status of ASL is in very early stages.

## License
