#ifndef SHADER_H_
#define SHADER_H_

#include "vectorlib.h"
#include <cglm/cglm.h>

enum ShaderTypes { UNI_BOOL = 0, UNI_INT, UNI_FLOAT, UNI_VEC2, UNI_VEC3, UNI_VEC4, UNI_MAT2, UNI_MAT3, UNI_MAT4, UNI_SAMPLER1D, UNI_SAMPLER2D, UNI_SAMPLER3D};

/**
 * 
 */
typedef struct ShaderUniformObject{
	char *name;
	char *description;
	unsigned int uniform;
	enum ShaderTypes type;
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
	float min, max;
}ShaderUniformObject;

/**
 * 
 */
enum SHADER_STAGE{STAGE_UNDEFINED, STAGE_VERTEX, STAGE_FRAGMENT, STAGE_GEOMETRY, STAGE_TESSELLATE_CONT, STAGE_TESSELLATE_EVAL, STAGE_COMPUTE};

/**
 * 
 */
typedef struct ShaderStage{
	unsigned int gl_id;
	bool is_compiled;
	enum SHADER_STAGE stage_type;
	char *source;

	unsigned int num_uniforms;
	ShaderUniformObject *uniforms;
}ShaderStage;

/**
 * 
 */
typedef struct Shader{
	char *name; // TODO: Unsure yet whether to use a custom name or shader file path
	char *path;
	bool is_loaded;
	unsigned int id;

	unsigned int num_stages;
	ShaderStage *stages;

	unsigned int num_uniforms;
	ShaderUniformObject *uniforms;

	uint8_t num_texture_slots;
}Shader;

/**
 *  Active shader
 */
extern unsigned int current_shader;

/**
 * 
 */
Shader ShaderOpen(char *filename);

/**
 * 
 */
void ShaderFree(Shader *shader);

/**
 * 
 */
void ShaderPassUniforms(Shader *shader);

/**
 * 
 */
void ShaderSet(Shader *shader);

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