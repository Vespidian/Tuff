#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "shader.h"
#include "vectorlib.h"

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
		int _sampler1D;
		int _sampler2D;
		int _sampler3D;
	}value;

	enum UNIFORM_TYPE type;

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

/**
 *  @brief Load a material from a file (Material's shader must be specified using 'MaterialShaderSet()' function)
 *  @param path - Path to material file (*.mat)
 */
Material MaterialOpen(char *path);

/**
 *  @brief Reload the file specified in 'material->path' back into '*material'
 */
void MaterialReload(Material *material);

/**
 *  @brief Free a material
 */
void MaterialFree(Material *material);

/**
 *  @brief Find a uniform in a material by name
 */
MaterialUniform *MaterialUniformFind(Material *material, char *uniform_name);

/**
 *  @brief Clean up useless uniforms and mark exposed uniforms (defined by the shader) (Must be called AFTER 'MaterialShaderSet()')
 */
void MaterialUniformsValidate(Material *material);

/**
 *  @brief Set the material's parent shader to 'shader'
 */
void MaterialShaderSet(Material *material, Shader *shader);

/**
 *  @brief Pass the uniforms stored in 'material' to its parent shader for the values to be given to OpenGL (Essentially sets 'material' as the active material)
 */
void MaterialShaderPassUniforms(Material *material);


/**
 * UNIFORM SETTERS AND GETTERS 
 */

// Primitives
void MaterialUniformSetBool(Material *material, char *uniform_name, bool value);
void MaterialUniformSetInt(Material *material, char *uniform_name, int value);
void MaterialUniformSetFloat(Material *material, char *uniform_name, float value);

// Vectors (typedef)
void MaterialUniformSetVec2(Material *material, char *uniform_name, vec2 value);
void MaterialUniformSetVec3(Material *material, char *uniform_name, vec3 value);
void MaterialUniformSetVec4(Material *material, char *uniform_name, vec4 value);

// Vectors (manual)
void MaterialUniformSetVec2_m(Material *material, char *uniform_name, float x, float y);
void MaterialUniformSetVec3_m(Material *material, char *uniform_name, float x, float y, float z);
void MaterialUniformSetVec4_m(Material *material, char *uniform_name, float x, float y, float z, float w);

// Samplers
void MaterialUniformSetSampler1D(Material *material, char *uniform_name, int sampler);
void MaterialUniformSetSampler2D(Material *material, char *uniform_name, int sampler);
void MaterialUniformSetSampler3D(Material *material, char *uniform_name, int sampler);


#endif