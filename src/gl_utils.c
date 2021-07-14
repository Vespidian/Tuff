#include "global.h"
#include "debug.h"
#include "gl_context.h"
#include "renderer/renderer.h"

#include "gl_utils.h"

// Active opengl references
unsigned int current_shader;
unsigned int current_vao;
unsigned int bound_textures[16];
unsigned int current_texture_unit;

TextureObject undefined_texture;

void InitGLUtils(){
	undefined_texture = LoadTexture("../images/undefined.png", GL_RGBA);
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

void LoadShader(const char *filename, unsigned int *vertex, unsigned int *fragment){
    SDL_RWops *shader_file;
    char *shader_source;
    int file_size;

    // Read shader source from file
    shader_file = SDL_RWFromFile(filename, "rb");
    if(shader_file == NULL){
        DebugLog(D_ERR, "Shader loading error! Shader file: %s not found", filename);
        printf("Shader loading error! Shader file: %s not found", filename);
        return;
    }

    // Find length of shader source file
    file_size = SDL_RWseek(shader_file, 0, SEEK_END);
    SDL_RWseek(shader_file, 0, SEEK_SET);

    // Allocate space for shader string
    shader_source = (char*)malloc(sizeof(char) * (file_size + 2));

    // Read shader to string and null terminate it
    SDL_RWread(shader_file, shader_source, 1, file_size);
    shader_source[file_size + 1] = '\0';

	// Split string into seperate strings for each shader type
	char *vert_string;
	char *frag_string;
	char shader_type_string[32] = {0}; // String containing the name of the current shader
	char *header_offset = shader_source; // String pointer for offseting through the 'shader_source' string
	do{
		// Find the start of the next shader
		header_offset = strstr(header_offset, "@shader ");

		// Make sure there is a shader left to find
		if(header_offset == NULL){
			break;
		}

		// Set the offset pointer to the start of the shader type
		header_offset += 8 * sizeof(char);

		// Copy the shader type into the 'shader_type_string'
		memcpy(shader_type_string, header_offset, strchr(header_offset, '\n') - header_offset - 1);

		// Find the end of the current shader (either the beginning of the next shader or the end of file)
		char *shader_end = strstr(header_offset, "@shader ");
		if(shader_end == NULL){
			shader_end = shader_source + file_size + 1;
		}

		// Set the offset to the beginning of this shader (Jump over the shader type)
		header_offset += strchr(header_offset, '\n') - header_offset;

		// Copy shaders into corresponding strings
		if(strcmp(shader_type_string, "vertex") == 0){
			vert_string = malloc(shader_end - header_offset);
			memcpy(vert_string, header_offset, shader_end - header_offset - 1);
			vert_string[shader_end - header_offset - 1] = 0; // Null terminate the shader string

		}else if(strcmp(shader_type_string, "fragment") == 0){
			frag_string = malloc(shader_end - header_offset);
			memcpy(frag_string, header_offset, shader_end - header_offset - 1);
			frag_string[shader_end - header_offset - 1] = 0; // Null terminate the shader string

		}else{
			DebugLog(D_WARN, "Error unknown shader type '%s' specified in file '%s'", shader_type_string, filename);
		}

		// Reset the shader type string
		memset(shader_type_string, 0, sizeof(shader_type_string));
	}while(header_offset != NULL);

    // Tell opengl to compile the shader

	// FRAGMENT SHADER
    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	int length = strlen(frag_string);
    GLCall(glShaderSource(fragment_shader, 1, (const GLchar*const*)&frag_string, &length));
    GLCall(glCompileShader(fragment_shader));

	free(frag_string);

    // Check for errors
    int success;
    GLCall(glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success));
    if(!success){
        char info_log[1024];
        GLCall(glGetShaderInfoLog(fragment_shader, 1024, NULL, info_log));
        DebugLog(D_ERR, "Shader: '%s' compilation failed: %s", filename, info_log);
        printf("Shader: '%s' compilation failed: %s", filename, info_log);
    }
	
	// VERTEX SHADER
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	length = strlen(vert_string);
    GLCall(glShaderSource(vertex_shader, 1, (const GLchar*const*)&vert_string, &length));
    GLCall(glCompileShader(vertex_shader));

	free(vert_string);

    // Check for errors
    GLCall(glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success));
    if(!success){
        char info_log[1024];
        GLCall(glGetShaderInfoLog(vertex_shader, 1024, NULL, info_log));
        DebugLog(D_ERR, "Shader: '%s' compilation failed: %s", filename, info_log);
        printf("Shader: '%s' compilation failed: %s", filename, info_log);
    }

	// Free variables
    SDL_RWclose(shader_file);
    free(shader_source);
	
	*vertex = vertex_shader;
	*fragment = fragment_shader;
}

Uint32 LoadShaderProgram(char *shader_file){
    Uint32 shader_program;
    unsigned int v, f;
    // v = LoadShader(GL_VERTEX_SHADER, vertex_shader_file);
    // f = LoadShader(GL_FRAGMENT_SHADER, fragment_shader_file);
	// printf("v before: %u\n", v);
	LoadShader(shader_file, &v, &f);

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

void SetVAO(unsigned int vao){
	if(current_vao != vao){
		glBindVertexArray(vao);
		current_vao = vao;
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

TextureObject LoadTexture(const char *path, int format){
    unsigned int texture;
    SDL_Surface *tmp_surface;
    
    GLCall(glGenTextures(1, &texture));
    GLCall(glBindTexture(GL_TEXTURE_2D, texture));

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    // GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    tmp_surface = IMG_Load(path);
	if(tmp_surface == NULL){
		DebugLog(D_ERR, "Error reading image: '%s'", path);
		return undefined_texture;
	}
    InvertSurfaceVertical(tmp_surface);

    TextureObject tex_out = {texture, tmp_surface->w, tmp_surface->h};
    if(tmp_surface){
		int internal = GL_RGBA8;
		if(format == GL_RGB){
			internal = GL_RGB8;
		}
        // GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tmp_surface->w, tmp_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp_surface->pixels));
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, internal, tmp_surface->w, tmp_surface->h, 0, format, GL_UNSIGNED_BYTE, tmp_surface->pixels));
        GLCall(glGenerateMipmap(GL_TEXTURE_2D));
		DebugLog(D_ACT, "Loaded texture: '%s'", path);
    }else{
        DebugLog(D_ERR, "Could not load image: '%s'", path);
		tex_out = undefined_texture;
    }
    SDL_FreeSurface(tmp_surface);
    return tex_out;
}

TilesheetObject LoadTilesheet(const char *path, int format, int tile_width, int tile_height){
    return (TilesheetObject){0, LoadTexture(path, format), tile_width, tile_height};
}
