#ifndef GL_UTILS_H_
#define GL_UTILS_H_

void InitGLUtils();

#include "vectorlib.h"
#include "include/cglm/cglm.h"

enum ShaderTypes { UNI_BOOL = 0, UNI_INT, UNI_FLOAT, UNI_VEC2, UNI_VEC3, UNI_VEC4, UNI_MAT2, UNI_MAT3, UNI_MAT4, UNI_SAMPLER1D, UNI_SAMPLER2D, UNI_SAMPLER3D };
typedef struct ShaderUniformObject{
	char *name;
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
	};
}ShaderUniformObject;

typedef struct ShaderObject{
	char *name; // Unsure yet whether to use a custom name or shader file path
	unsigned int id;
	ShaderUniformObject *uniforms;
	unsigned int num_uniforms;
	bool using_texture_slot[16];
}ShaderObject;
ShaderObject ParseShaderUniforms(char *name, unsigned int id, char *vertex, char *fragment);

/**
 *  Active shader
 */
extern unsigned int current_shader;

/**
 *  Active VAO
 */
extern unsigned int current_vao;

/**
 *  Ordered array of all bound textures
 */
extern unsigned int bound_textures[16];

/**
 *  Bound texture unit
 */
extern unsigned int current_texture_unit;

void CheckGLErrors(const char *file, int line);
ShaderObject LoadShaderProgram(char *filename);

void PassShaderUniforms(ShaderObject *shader);
void SetShaderProgram(ShaderObject *shader);
void SetVAO(unsigned int vao);

void UniformSetBool(ShaderObject *shader, char *uniform_name, bool value);
void UniformSetInt(ShaderObject *shader, char *uniform_name, int value);
void UniformSetFloat(ShaderObject *shader, char *uniform_name, float value);

// Vectors (datatype)
void UniformSetVec2(ShaderObject *shader, char *uniform_name, vec2 value);
void UniformSetVec3(ShaderObject *shader, char *uniform_name, vec3 value);
void UniformSetVec4(ShaderObject *shader, char *uniform_name, vec4 value);

// Vectors (manual)
void UniformSetVec2_m(ShaderObject *shader, char *uniform_name, float x, float y);
void UniformSetVec3_m(ShaderObject *shader, char *uniform_name, float x, float y, float z);
void UniformSetVec4_m(ShaderObject *shader, char *uniform_name, float x, float y, float z, float w);

// Matrix
void UniformSetMat2(ShaderObject *shader, char *uniform_name, mat2 mat);
void UniformSetMat3(ShaderObject *shader, char *uniform_name, mat3 mat);
void UniformSetMat4(ShaderObject *shader, char *uniform_name, mat4 mat);

// Samplers
void UniformSetSampler1D(ShaderObject *shader, char *uniform_name, int sampler);
void UniformSetSampler2D(ShaderObject *shader, char *uniform_name, int sampler);
void UniformSetSampler3D(ShaderObject *shader, char *uniform_name, int sampler);

int InvertSurfaceVertical(SDL_Surface *surface);

typedef struct TextureObject{
    unsigned int gl_tex;
    int w;
    int h;
}TextureObject;

typedef struct TilesheetObject{
	unsigned int id;
    TextureObject texture;
    int tile_w;
    int tile_h;
}TilesheetObject;

/**
 *  Undefined texture to use in case of error
 */
extern TextureObject undefined_texture;
TextureObject LoadTexture(const char *path);
TilesheetObject LoadTilesheet(const char *path, int tile_width, int tile_height);

#endif