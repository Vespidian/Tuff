#ifndef GL_UTILS_H_
#define GL_UTILS_H_

void InitGLUtils();

void CheckGLErrors(const char *file, int line);
Uint32 LoadShaderProgram(const char *vertex_shader_file, const char *fragment_shader_file);
void SetShaderProgram(unsigned int shader);

void UniformSetBool(Uint32 program, const char *uniform_name, bool value);
void UniformSetInt(Uint32 program, const char *uniform_name, int value);
void UniformSetFloat(Uint32 program, const char *uniform_name, float value);

// Vectors (datatype)
void UniformSetVec2(Uint32 program, const char *uniform_name, vec2 value);
void UniformSetVec3(Uint32 program, const char *uniform_name, vec3 value);
void UniformSetVec4(Uint32 program, const char *uniform_name, vec4 value);

// Vectors (manual)
void UniformSetVec2_m(Uint32 program, const char *uniform_name, float x, float y);
void UniformSetVec3_m(Uint32 program, const char *uniform_name, float x, float y, float z);
void UniformSetVec4_m(Uint32 program, const char *uniform_name, float x, float y, float z, float w);

// Matrix
void UniformSetMat2(Uint32 program, const char *uniform_name, mat2 mat);
void UniformSetMat3(Uint32 program, const char *uniform_name, mat3 mat);
void UniformSetMat4(Uint32 program, const char *uniform_name, mat4 mat);

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

extern TextureObject undefined_texture;
TextureObject LoadTexture(const char *path);
TilesheetObject LoadTilesheet(const char *path, int tile_width, int tile_height);

#endif