#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "shader.h"
#include "vectorlib.h"

// enum PARAMETER{ UNI_BOOL = 0, UNI_INT, UNI_FLOAT, UNI_VEC2, UNI_VEC3, UNI_VEC4, UNI_SAMPLER1D, UNI_SAMPLER2D, UNI_SAMPLER3D};

typedef struct MaterialUniform{
	char *uniform_name;

	char *texture_path;

	union{
		bool _bool;
		int _int;
		float _float;
		Vector2 _vec2;
		Vector3 _vec3;
		Vector4 _vec4;

		// SAMPLERS GO HERE
		int _sampler;
	}value;

}MaterialUniform;

typedef struct Material{
	char *path;
	char *name;

	bool is_validated;
	bool is_loaded;

	char *shader_path;
	Shader *shader;

	unsigned int num_uniforms;
	MaterialUniform *uniforms;
}Material;


Material MaterialOpen(char *path);
void MaterialFree(Material *material);
MaterialUniform *MaterialUniformGet(Material *material, char *uniform_name);
void MaterialUniformsValidate(Material *material);
void MaterialSetShader(Material *material, Shader *shader);
void MaterialShaderSet(Material *material);

/*
	=== PLAN ===

	Start by finding the shader (deal with failure case) and setting the shader pointer
	-> Every time we use the material, we check 'shader_path' against 'shader->path' to make sure the correct shader is loaded

	Copy the shader's uniforms over to the material
	(only the exposed uniforms)

	Now uniforms are edited per material.

	When we want to render a mesh with a material, we copy the material's uniforms back to the shader
	and send the uniforms to the shader program (ShaderPassUniforms)
	-> The function that copies the data into the shader should check if the value of a uniform changes to keep track of OpenGL state
		(if the value does not change we dont want to upload the uniform again)

	When reloading a shader, the material also needs to be reloaded to gain access to any new uniforms

	All of the above assumes the uniform array never changes once the shader is loaded in
*/

#endif