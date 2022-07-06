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
MaterialUniform *MaterialUniformGet(Material *material, char *uniform_name);

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

#endif