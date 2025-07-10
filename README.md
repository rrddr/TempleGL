This is a simple rendering engine written using modern OpenGL and C++ features, written as a learning project. 

In particular, I use [Direct State Access](https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions) ("Named" functions), instead of oldschool binding/unbinding with VAO's etc. 
I also use [Multi-Draw Indirect](https://ktstephano.github.io/rendering/opengl/mdi) to reduce draw calls (the slowest part of traditional OpenGL) by using command buffers.
Moreover, I use [Debug Message Events](https://www.khronos.org/opengl/wiki/Debug_Output) instead of `glGetError()` style error checking. This has a fairly big performance impact,
and can be disabled in the `config.yaml` file.

However, as this is my first attempt at writing an OpenGL app from scratch (i.e. not following any tutorials), I made many poor architectural decisions, which led me to abandon the
project.

I use model data extracted from a Minecraft save using [jmc2obj](https://github.com/jmc2obj/j-mc-2-obj). For texture data I used the [Vanilla PBR](https://www.curseforge.com/minecraft/texture-packs/vanilla-pbr) pack, 
which includes separate diffuse, normal and specular textures (however, I realised at some point that I interpret this data incorrectly, see [here](https://shaderlabs.org/wiki/LabPBR_Material_Standard)).
I got the skybox from [here](https://freestylized.com/skybox/sky_clouds_09/).

# Features

- Pre-processing for GLSL shader code, with support for #include directives, and for passing constants defined in the C++ code (mainly used for binding values, so that I only have to specify them once).
- Cascaded Shadow Mapping for sunlight-shadows, with a configurable number of cascades. I use the
[Practical Split Scheme](https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-10-parallel-split-shadow-maps-programmable-gpus)
algorithm to determine where to split the view frustrum.
- Normal and specular mapping, with a simple Blinn-Phong shader. The code for this is very dirty and inefficient, as I meant to replace it with a physically-based renderer at some point.
Point lights are also implemented in the most naive way, as I was working on implementing [clustered shading](https://www.aortiz.me/2018/12/21/CG.html), and wanted to see how much of an improvement this would be.
(I have implementations for both of these things in separate projects, but those are based on tutorials, so I did not want to copy-paste them).
- HDR rendering, with tone-mapping (and gamma-correction) in a separate screen-space pass.
- Standard WASD + Mouse camera controls (+ Shift/Space to go down/up, and scroll-wheel to adjust move speed).

<img width="1921" alt="CSM-screenshot" src="https://github.com/rrddr/TempleGL/blob/main/CSMexample1.png" title="Close and distant shadows of similar quality.">
