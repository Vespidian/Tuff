#ifndef SHADER_H_
#define SHADER_H_

#include "vectorlib.h"
#include <cglm/cglm.h>

enum UNIFORM_TYPE{ UNI_BOOL = 0, UNI_INT, UNI_FLOAT, UNI_VEC2, UNI_VEC3, UNI_VEC4, UNI_MAT2, UNI_MAT3, UNI_MAT4, UNI_SAMPLER1D, UNI_SAMPLER2D, UNI_SAMPLER3D};

/**
 *  @brief Shader uniform containing value, name and other flags
 */
typedef struct ShaderUniform{
	// Uniform's name straight from the shader's source code
	char *name;

	// Description of the uniform (set in the .shader file)
	char *description;

	// OpenGL ID of the uniform
	int uniform;

	// Whether or not the current value matches the OpenGL contexts value
	bool is_uploaded;

	// Uniform type (float, vec3, ..)
	enum UNIFORM_TYPE type;
	union value{
		bool _bool;
		int _int;
		float _float;

		Vector2 _vec2;
		Vector3 _vec3;
		Vector4 _vec4;

		mat2 _mat2;
		mat3 _mat3;
		mat4 _mat4;

		int _sampler1d;
		int _sampler2d;
		int _sampler3d;
	}value;

	// Whether or not this uniform has defined value limits and a default value
	bool is_exposed;

	union{
		bool _bool;
		int _int;
		float _float;
		Vector2 _vec2;
		Vector3 _vec3;
		Vector4 _vec4;
	}value_default;


	// union{
	// 	int _int;
	// 	float _float;
	// }min;

	// union{
	// 	int _int;
	// 	float _float;
	// }max;
	bool has_range;
	float min, max;
}ShaderUniform;

/**
 *  @brief Shader stage type
 */
enum SHADER_STAGE{STAGE_UNDEFINED, STAGE_VERTEX, STAGE_FRAGMENT, STAGE_GEOMETRY, STAGE_TESSELLATE_CONT, STAGE_TESSELLATE_EVAL, STAGE_COMPUTE};

/**
 *  @brief Shader stage context containing information on a single stage of a shader program
 */
typedef struct ShaderStage{
	// Stage's OpenGL ID
	unsigned int gl_id;

	// Whether or not the stage was successfully compiled
	bool is_compiled;

	// Type of stage (FRAG, VERT, ..)
	enum SHADER_STAGE stage_type;

	// String containing shader stage's source code
	char *source;

	// Number of uniforms found in stage source
	unsigned int num_uniforms;

	// Stage + Exposed uniforms
	ShaderUniform *uniforms;
}ShaderStage;

/**
 *  @brief Shader context containing information on an entire shader program
 */
typedef struct Shader{
	// Descriptive memorable name defined in .shader file
	char *name;

	// Path to .shader file
	char *path;

	// Whether or not the shader was correctly loaded
	bool is_loaded;

	// Shader's OpenGL ID
	unsigned int id;

	// Number of shader stages within the shader
	unsigned int num_stages;

	// Array of shader stages
	ShaderStage *stages;

	// Number of unique uniforms throughout all shader stages
	unsigned int num_uniforms;

	// Array of unique uniforms from all stages
	ShaderUniform *uniforms;

	// Number of texture slots the shader makes use of
	uint8_t num_texture_slots;
}Shader;

/**
 *  Active shader (in the OpenGL state)
 *  ( Must be set by the renderer )
 */
extern unsigned int current_shader;

/**
 *  @brief Open, parse, and compile the specified shader file
 *  @param path - Path to file to be read
 *  @return A Shader ready to be used
 */
Shader ShaderOpen(char *filename);

/**
 *  @brief Free a shader
 *  @param shader Pointer to the shader to be freed
 */
void ShaderFree(Shader *shader);

/**
 *  @brief Set the active shader in the OpenGL context - 'current_shader' is the currently active shader
 */
void ShaderSet(Shader *shader);

/**
 *  @brief Send the specified shader's uniforms to the OpenGL context
 *  @param shader - Shader whose uniforms are to be sent to OpenGL
 */
void ShaderPassUniforms(Shader *shader);

/**
 * 	@brief Retrieve the index of a uniform by name from 'shader'
 * 	@return Index of 'shader.uniforms[]' or -1 upon error (uniform name doesnt exist)
 */
int ShaderUniformFind(Shader *shader, char *name);


// Primitives
void UniformSetBool(Shader *shader, char *uniform_name, bool value);
void UniformSetInt(Shader *shader, char *uniform_name, int value);
void UniformSetFloat(Shader *shader, char *uniform_name, float value);

// Vectors (typedef)
void UniformSetVec2(Shader *shader, char *uniform_name, vec2 value);
void UniformSetVec3(Shader *shader, char *uniform_name, vec3 value);
void UniformSetVec4(Shader *shader, char *uniform_name, vec4 value);

// Vectors (manual)
void UniformSetVec2_m(Shader *shader, char *uniform_name, float x, float y);
void UniformSetVec3_m(Shader *shader, char *uniform_name, float x, float y, float z);
void UniformSetVec4_m(Shader *shader, char *uniform_name, float x, float y, float z, float w);

// Matrix
void UniformSetMat2(Shader *shader, char *uniform_name, mat2 mat);
void UniformSetMat3(Shader *shader, char *uniform_name, mat3 mat);
void UniformSetMat4(Shader *shader, char *uniform_name, mat4 mat);

// Samplers
void UniformSetSampler1D(Shader *shader, char *uniform_name, int sampler);
void UniformSetSampler2D(Shader *shader, char *uniform_name, int sampler);
void UniformSetSampler3D(Shader *shader, char *uniform_name, int sampler);


#endif