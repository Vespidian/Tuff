# Tuff Shader Loader
Loader of custom shaders for the [Tuff game engine](https://github.com/Vespidian/GameFramework).

## SHADERS (.shader)
A shader is code to be given to the GPU to render triangles in a certain way.
In .shader files, public uniforms that materials can access are also specified.

Requires both a fragment and vertex stage

### Each stage supports the following subsections:
- stage 	(The type of shader (VERTEX, FRAGMENT, ...))
- source 	(The source code of the shader)
- uniforms	(The uniforms to be exposed to any materials using this shader)

### Uniforms support each of the following subsections:
- uniform	(Uniform to reference in the source code)
- name		(Custom name to be displayed in editor)
- default	(Default value to be assigned to materials only for certain types (int, float, bool, vec1/2/3))
- range		(Allowable range for integer and floating point values)

#### [Example Base Shader](assets/default.shader) - See also [all the test shaders](assets/)
