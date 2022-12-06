# Elever Render
## A multi-platform vendor independent photorealistic rendering engine

![alt text](https://i.imgur.com/G7ExwxZ.png)
*Scene rendered with Eleven. 1000 samples and denoised*

Eleven Render is my personal project. It's a unidirectional C++ path tracing rendering engine, where compatibility prevails over other aspects. Thanks to SYCL, it's possible to compile it for almost any GPU/CPU.
It also seeks for broad compatibility with 3D Model software thanks to ASL, a shading language I'm working on, which allows other shading languages (like OSL), to be converted to it.

At the moment, Eleven is a WIP, and features like ASL are just proof of concepts. 


## Releases
The only way to use Eleven Render right now is through Blender, thanks to the [ElevenBlender plug-in](https://github.com/101001000/ElevenBlender) 
Download the last ElevenBlender release here: <span style="color:red">**There's no releases yet**</span>.


## Features

- Compatibility with both GPU and CPU (vendor independent)
- Normal recalculation with Vertex Weighted algorithm (so if your model has broken normals you won't have to worry about)
- Osl compatibility through a custom shading language
- MIS (BRDF, Environment, PointLights)
- Denoising (Open Image Denoiser) 

## Build

**Disclaimer** At the current moment, there's not a builiding guide which will allow you to build Eleven flawlessly. I plan to release a Linux one, and to solve some compilation issues.

I'm also working on installing Boost libraries automatically, but [there's some compatibility issues with Windows, Intel DPC++ and Boost](https://github.com/101001000/ElevenRender/issues/7)

### Windows (VS2022) - CUDA

Install [Boost 1.80.00](https://www.boost.org/doc/libs/1_62_0/more/getting_started/windows.html) on the system (C:\Program Files\boost\boost_1_80_0), and compile it following the step 5 from the linked instructions. Ideally you must compile it with clangw16 toolset. 

Currently, the 16 version of Clang, is not available on Visual Studio, so if you want to avoid the mess to setup Visual Studio for clangw16, you can apply this quick dirty patch[^1]:
You need to override the BOOST_LIB_TOOLSET variable, located in C:\Program Files\boost\boost_1_80_0\boost\config\auto_link.hpp to "clangw16", for example by typing #define BOOST_LIB_TOOLSET "clangw15" in the line 227.

Finally run build.bat. It will download automatically the rest of the dependencies, including the DPC++ Intel's toolchain. 

<!--- If you want to use your own SYCL implementation, you can just run build.bat with an additional argument pointing your sycl_workspace installation.)-->



[^1]: I was not able to compile Boost with the Intel's DPC++ toolchain, because it doesn't support Windows SEH exceptions, which are being used in the Boost::asio library. To solve it, I made a quick patch by compiling Boost manually and renaming some of the library files. I'm working on being able to compile Boost with CMAKE, meanwhile, that's the solution.


## Docs

<details open>
<summary>Third party libraries used</summary>

- [SYCL]() 
- [Boost](https://github.com/boostorg/boost) for TCP, logging, program args, 
- [Intel Open Image Denoiser](https://github.com/OpenImageDenoise/oidn) for final denoise
- [Flex](https://github.com/westes/flex) for ASL parsing
- [Bison](https://github.com/akimd/bison) for ASL parsing
- [Mikktspace](https://github.com/mmikk/MikkTSpace) for generating the tangent space
- [Rapidobj](https://github.com/guybrush77/rapidobj) for loading wavefront objects
- [stb_image & stb_image_write](https://github.com/nothings/stb) for image IO


<br>
</details>

## Disclaimer

Eleven Render is the result of my Bachelor's Thesis and now my Master's Thesis. The way I worked on it was, to get the most features, in the smallest possible time, so most of the code is messy and unorganized. I'm working on modernizing the code and cleaning it up, as well as documenting it. Most of features are incomplete. The shading is giving me issues, so the implementation is probably not correct. The efficiency is also not the best and it's full of memory leaks. I hope to have all this sorted out in the following months, but for now, this is the render's state.

## ASL
Eleven Render runs a custom formally specified shading language called Abstract Shading Language. ASL tries to unify shading languages, acting as middle step between shading code and Eleven. The current implementation, just covers an extremely inefficient intepretation of such a language. Compilation is planned in the future, but the current status of ASL is in very early stages.

## License
