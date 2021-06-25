#include "global.h"
#include "debug.h"
#include "gl_context.h"
#include "renderer/renderer.h"

#include "gl_utils.h"

TextureObject undefined_texture;

void InitGLUtils(){
	undefined_texture = LoadTexture("../images/undefined.png");
}

void CheckGLErrors(const char *file, int line){
    unsigned int error_code;
    while((error_code = glGetError()) != GL_NO_ERROR){
        char error_string[32];
        switch(error_code){
            case 1280:
                strcpy(error_string, "INVALID_ENUM");
                break;
            case 1281:
                strcpy(error_string, "INVALID_VALUE");
                break;
            case 1282:
                strcpy(error_string, "INVALID_OPERATION");
                break;
            case 1283:
                strcpy(error_string, "STACK_OVERFLOW");
                break;
            case 1284:
                strcpy(error_string, "STACK_UNDERFLOW");
                break;
            case 1285:
                strcpy(error_string, "OUT_OF_MEMORY");
                break;
            case 1286:
                strcpy(error_string, "INVALID_FRAMEBUFFER_OPERATION");
                break;
			default:
				break;
        }
        DebugLog(D_ERR, "Opengl error '%s' ('%s': %d)", error_string, file, line);
        printf("Opengl error '%s' ('%s': %d)", error_string, file, line);
    }
}

Uint32 LoadShader(Uint32 shader_type, const char *filename){
    SDL_RWops *shader_file;
    Uint32 shader;
    char *shader_source;
    int file_size;

    // Read shader source from file
    shader_file = SDL_RWFromFile(filename, "rb");
    if(shader_file == NULL){
        DebugLog(D_ERR, "Shader loading error! Shader file: %s not found", filename);
        printf("Shader loading error! Shader file: %s not found", filename);
        return 0;
    }
    // Find length of shader source file
    file_size = SDL_RWseek(shader_file, 0, SEEK_END);
    SDL_RWseek(shader_file, 0, SEEK_SET);

    // Allocate space for shader string
    shader_source = (char*)malloc(sizeof(char) * (file_size + 2));
    // Read shader to string and null terminate it
    SDL_RWread(shader_file, shader_source, 1, file_size);
    shader_source[file_size + 1] = '\0';

    // Tell opengl to comile the shader
    shader = glCreateShader(shader_type);
    GLCall(glShaderSource(shader, 1, (const GLchar*const*)&shader_source, &file_size));
    GLCall(glCompileShader(shader));

    // Free variables
    SDL_RWclose(shader_file);
    free(shader_source);

    // Check for errors
    int success;
    GLCall(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if(!success){
        char info_log[1024];
        GLCall(glGetShaderInfoLog(shader, 1024, NULL, info_log));
        DebugLog(D_ERR, "Shader: '%s' compilation failed: %s", filename, info_log);
        printf("Shader: '%s' compilation failed: %s", filename, info_log);
        return 0;
    }

    return shader;
}

Uint32 LoadShaderProgram(const char *vertex_shader_file, const char *fragment_shader_file){
    Uint32 shader_program;
    Uint32 v, f;
    v = LoadShader(GL_VERTEX_SHADER, vertex_shader_file);
    f = LoadShader(GL_FRAGMENT_SHADER, fragment_shader_file);

    shader_program = glCreateProgram();
    GLCall(glAttachShader(shader_program, v));
    GLCall(glAttachShader(shader_program, f));
    GLCall(glLinkProgram(shader_program));

    int success;
    GLCall(glGetProgramiv(shader_program, GL_LINK_STATUS, &success));
    if(!success){
        char info_log[1024];
        GLCall(glGetProgramInfoLog(shader_program, 1024, NULL, info_log));
        DebugLog(D_ERR, "Shader linking failed: %s", info_log);
        printf("Shader linking failed: %s", info_log);
    }

    GLCall(glDeleteShader(v));
    GLCall(glDeleteShader(f));

    return shader_program;
}

void SetShaderProgram(unsigned int shader){
	if(current_shader != shader){
		glUseProgram(shader);
		current_shader = shader;
	}
}

void UniformSetBool(Uint32 program, const char *uniform_name, bool value){
	SetShaderProgram(program);
    glUniform1i(glGetUniformLocation(program, uniform_name), (int)value);
}

void UniformSetInt(Uint32 program, const char *uniform_name, int value){
	SetShaderProgram(program);
    glUniform1i(glGetUniformLocation(program, uniform_name), value);
}

void UniformSetFloat(Uint32 program, const char *uniform_name, float value){
	SetShaderProgram(program);
    glUniform1i(glGetUniformLocation(program, uniform_name), value);
}

// Vector (datatype)
void UniformSetVec2(Uint32 program, const char *uniform_name, vec2 value){
	SetShaderProgram(program);
    glUniform2fv(glGetUniformLocation(program, uniform_name), 1, value);
}
void UniformSetVec3(Uint32 program, const char *uniform_name, vec3 value){
	SetShaderProgram(program);
    glUniform3fv(glGetUniformLocation(program, uniform_name), 1, value);
}
void UniformSetVec4(Uint32 program, const char *uniform_name, vec4 value){
	SetShaderProgram(program);
    glUniform4fv(glGetUniformLocation(program, uniform_name), 1, value);
}
// Vector (manual)
void UniformSetVec2_m(Uint32 program, const char *uniform_name, float x, float y){
	SetShaderProgram(program);
    glUniform2f(glGetUniformLocation(program, uniform_name), x, y);
}
void UniformSetVec3_m(Uint32 program, const char *uniform_name, float x, float y, float z){
	SetShaderProgram(program);
    glUniform3f(glGetUniformLocation(program, uniform_name), x, y, z);
}
void UniformSetVec4_m(Uint32 program, const char *uniform_name, float x, float y, float z, float w){
	SetShaderProgram(program);
    glUniform4f(glGetUniformLocation(program, uniform_name), x, y, z, w);
}

// Matrix
void UniformSetMat2(Uint32 program, const char *uniform_name, mat2 mat){
	SetShaderProgram(program);
    glUniformMatrix2fv(glGetUniformLocation(program, uniform_name), 1, GL_FALSE, mat[0]);
}
void UniformSetMat3(Uint32 program, const char *uniform_name, mat3 mat){
	SetShaderProgram(program);
    glUniformMatrix3fv(glGetUniformLocation(program, uniform_name), 1, GL_FALSE, mat[0]);
}
void UniformSetMat4(Uint32 program, const char *uniform_name, mat4 mat){
	SetShaderProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, uniform_name), 1, GL_FALSE, mat[0]);
}

int InvertSurfaceVertical(SDL_Surface *surface)
{
    Uint8 *t;
    register Uint8 *a, *b;
    Uint8 *last;
    register Uint16 pitch;

    /* do nothing unless at least two lines */
    if(surface->h < 2) {
        return 0;
    }

    /* get a place to store a line */
    pitch = surface->pitch;
    t = (Uint8*)malloc(pitch);

    if(t == NULL) {
        // SDL_UNLOCKIFMUST(surface);
        return -2;
    }

    /* get first line; it's about to be trampled */
    memcpy(t,surface->pixels,pitch);

    /* now, shuffle the rest so it's almost correct */
    a = (Uint8*)surface->pixels;
    last = a + pitch * (surface->h - 1);
    b = last;

    while(a < b) {
        memcpy(a,b,pitch);
        a += pitch;
        memcpy(b,a,pitch);
        b -= pitch;
    }

    /* in this shuffled state, the bottom slice is too far down */
    memmove( b, b+pitch, last-b );
    /* now we can put back that first row--in the last place */
    memcpy(last,t,pitch);

    free(t);

    return 0;
}

TextureObject LoadTexture(const char *path){
    unsigned int texture;
    SDL_Surface *tmp_surface;
    
    GLCall(glGenTextures(1, &texture));
    GLCall(glBindTexture(GL_TEXTURE_2D, texture));

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    tmp_surface = IMG_Load(path);
	if(tmp_surface == NULL){
		DebugLog(D_ERR, "Error reading image: '%s'", path);
		return undefined_texture;
	}
    InvertSurfaceVertical(tmp_surface);

    TextureObject tex_out = {texture, tmp_surface->w, tmp_surface->h};
	printf("%s\n", path);
    if(tmp_surface){
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tmp_surface->w, tmp_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp_surface->pixels));
        GLCall(glGenerateMipmap(GL_TEXTURE_2D));
		DebugLog(D_ACT, "Loaded texture: '%s'", path);
    }else{
        DebugLog(D_ERR, "Could not load image: '%s'", path);
		tex_out = undefined_texture;
    }
    SDL_FreeSurface(tmp_surface);
    return tex_out;
}

TilesheetObject LoadTilesheet(const char *path, int tile_width, int tile_height){
    return (TilesheetObject){0, LoadTexture(path), tile_width, tile_height};
}
