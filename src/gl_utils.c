#include "global.h"
#include "debug.h"
#include "gl_context.h"
#include "renderer/renderer.h"
#include "vectorlib.h"

#include "gl_utils.h"

// Active opengl references
unsigned int current_shader;
unsigned int current_vao;
unsigned int bound_textures[16];
unsigned int current_texture_unit;

TextureObject undefined_texture;

static const char shader_path[] = "../shaders/";

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

void PassShaderUniforms(ShaderObject *shader){
	SetShaderProgram(shader);

	for(int i = 0; i < shader->num_uniforms; i++){
		switch(shader->uniforms[i].type){
			case UNI_BOOL:
				glUniform1i(shader->uniforms[i].uniform, (int)shader->uniforms[i]._bool);
				break;
			case UNI_INT:
				glUniform1i(shader->uniforms[i].uniform, (int)shader->uniforms[i]._int);
				break;
			case UNI_FLOAT:
				glUniform1f(shader->uniforms[i].uniform, shader->uniforms[i]._float);
				break;

			case UNI_VEC2:
				glUniform2fv(shader->uniforms[i].uniform, 1, shader->uniforms[i]._vec2.v);
				break;
			case UNI_VEC3:
				glUniform3fv(shader->uniforms[i].uniform, 1, shader->uniforms[i]._vec3.v);
				break;
			case UNI_VEC4:
				glUniform4fv(shader->uniforms[i].uniform, 1, shader->uniforms[i]._vec4.v);
				break;

			case UNI_MAT2:
				glUniformMatrix2fv(shader->uniforms[i].uniform, 1, GL_FALSE, shader->uniforms[i]._mat2[0]);
				break;
			case UNI_MAT3:
				glUniformMatrix3fv(shader->uniforms[i].uniform, 1, GL_FALSE, shader->uniforms[i]._mat3[0]);
				break;
			case UNI_MAT4:
				glUniformMatrix4fv(shader->uniforms[i].uniform, 1, GL_FALSE, shader->uniforms[i]._mat4[0]);
				break;

			case UNI_SAMPLER1D:
				glUniform1i(shader->uniforms[i].uniform, shader->uniforms[i]._sampler1d);
				break;
			case UNI_SAMPLER2D:
				glUniform1i(shader->uniforms[i].uniform, shader->uniforms[i]._sampler2d);
				break;
			case UNI_SAMPLER3D:
				glUniform1i(shader->uniforms[i].uniform, shader->uniforms[i]._sampler3d);
				break;
		}
	}
}

ShaderObject ParseShaderUniforms(char *name, unsigned int id, char *vertex, char *fragment){
	ShaderObject shader = {.id = id};
	
	shader.name = malloc(sizeof(char) * strlen(name));
	strcpy(shader.name, name);

	shader.num_uniforms = 0;
	memset(shader.using_texture_slot, false, sizeof(bool) * 16);

	// --- CURRENTLY ONLY SCANS VERTEX STRING ---
	char *uniform_ptr = vertex;
	int num_samplers = 0;
	for(int i = 0; i < 2; i++){
		while((uniform_ptr = strstr(uniform_ptr, "uniform ")) != NULL){
			if(strchr(uniform_ptr, '{') < strchr(uniform_ptr, ';')){
				uniform_ptr += 8; // Increment by length of "uniform "
				continue;
			}

			shader.uniforms = realloc(shader.uniforms, sizeof(ShaderUniformObject) * (shader.num_uniforms + 1));
			uniform_ptr += 8; // Increment by length of "uniform "
			// Make sure this "uniform" keyword isnt referring to a uniform buffer

			// Determine the type of this uniform
			if(strncmp(uniform_ptr, "vec", 3) == 0){
				uniform_ptr += 3;
				switch((uniform_ptr)[0]){
					case '2':
						// printf("VEC2\n");
						memset(shader.uniforms[shader.num_uniforms]._vec2.v, 0, sizeof(Vector2));
						shader.uniforms[shader.num_uniforms].type = UNI_VEC2;
						break;
					case '3':
						// printf("VEC3\n");
						memset(shader.uniforms[shader.num_uniforms]._vec3.v, 0, sizeof(Vector2));
						shader.uniforms[shader.num_uniforms].type = UNI_VEC3;
						break;
					case '4':
						// printf("VEC4\n");
						memset(shader.uniforms[shader.num_uniforms]._vec4.v, 0, sizeof(Vector2));
						shader.uniforms[shader.num_uniforms].type = UNI_VEC4;
						break;
				}
				uniform_ptr += 1;
			}else if(strncmp(uniform_ptr, "mat", 3) == 0){
				uniform_ptr += 3;
				switch((uniform_ptr)[0]){
					case '2':
						// printf("MAT2\n");
						memset(shader.uniforms[shader.num_uniforms]._mat2, 0, sizeof(mat2));
						shader.uniforms[shader.num_uniforms].type = UNI_MAT2;
						break;
					case '3':
						// printf("MAT3\n");
						memset(shader.uniforms[shader.num_uniforms]._mat3, 0, sizeof(mat3));
						shader.uniforms[shader.num_uniforms].type = UNI_MAT3;
						break;
					case '4':
						// printf("MAT4\n");
						memset(shader.uniforms[shader.num_uniforms]._mat4, 0, sizeof(mat4));
						shader.uniforms[shader.num_uniforms].type = UNI_MAT4;
						break;
				}
				uniform_ptr += 1;
			}else if(strncmp(uniform_ptr, "sampler", 7) == 0){
				uniform_ptr += 7;
				shader.using_texture_slot[num_samplers++] = true;
				switch((uniform_ptr)[0]){
					case '1':
						shader.uniforms[shader.num_uniforms].type = UNI_SAMPLER1D;
						break;
					case '2':
						shader.uniforms[shader.num_uniforms].type = UNI_SAMPLER2D;
						break;
					case '3':
						shader.uniforms[shader.num_uniforms].type = UNI_SAMPLER3D;
						break;
				}
				uniform_ptr += 2;
			}else{
				shader.uniforms[shader.num_uniforms]._float = 0;
				if(strncmp(uniform_ptr, "float", 5) == 0){
					// printf("FLOAT\n");
					uniform_ptr += 5;
					shader.uniforms[shader.num_uniforms].type = UNI_FLOAT;
				}else if(strncmp(uniform_ptr, "int", 3) == 0){
					// printf("INT\n");
					uniform_ptr += 3;
					shader.uniforms[shader.num_uniforms].type = UNI_INT;
				}else if(strncmp(uniform_ptr, "bool", 4) == 0){
					// printf("BOOL\n");
					uniform_ptr += 4;
					shader.uniforms[shader.num_uniforms].type = UNI_BOOL;
				}
			}
			uniform_ptr += 1;

			// Get the name of the uniform
			int uniform_name_length = strchr(uniform_ptr, ';') - uniform_ptr;
			shader.uniforms[shader.num_uniforms].name = malloc(sizeof(char) * (uniform_name_length + 1));

			// Copy section of string
			memcpy(shader.uniforms[shader.num_uniforms].name, uniform_ptr, uniform_name_length);
			shader.uniforms[shader.num_uniforms].name[uniform_name_length] = 0;

			shader.uniforms[shader.num_uniforms].uniform = glGetUniformLocation(shader.id, shader.uniforms[shader.num_uniforms].name);

			// printf("uniform[%d].name = %s, .type = %d\n", shader.num_uniforms, shader.uniforms[shader.num_uniforms].name, shader.uniforms[shader.num_uniforms].type);
		
			// Make sure this isnt a copy of a uniform from another shader type within this shader program
			bool found_copy = false;
			for(int j = 0; j < shader.num_uniforms; j++){
				if(strcmp(shader.uniforms[j].name, shader.uniforms[shader.num_uniforms].name) == 0){
					found_copy = true;
					break;
				}
			}

			// If this is a copy, remove it
			if(found_copy){
				free(shader.uniforms[shader.num_uniforms].name);
				shader.uniforms = realloc(shader.uniforms, sizeof(ShaderUniformObject) * (shader.num_uniforms));
			}else{
				shader.num_uniforms++;
			}

		}
		uniform_ptr = fragment; // Do all that over again for the fragment shader
	}
	return shader;
}

ShaderObject LoadShaderProgram(char *filename){
    SDL_RWops *shader_file;
    char *shader_source;
    int file_size;

	ShaderObject shader_program;
    unsigned int vertex, fragment;

    // Read shader source from file
	char *path = malloc(sizeof(char) * (strlen(shader_path) + strlen(filename) + 1));
	strcpy(path, shader_path);
	strcat(path, filename);
	path[strlen(shader_path) + strlen(filename)] = 0;
	
    shader_file = SDL_RWFromFile(path, "rb");
    if(shader_file == NULL){
        DebugLog(D_ERR, "Shader loading error! Shader file: %s not found", path);
        printf("Shader loading error! Shader file: %s not found", path);
		// SHOULD EXIT THE FUNCTION HERE
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
	char *shader_strings[2];
	// char *vert_string;
	// char *frag_string;
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
		int offset_amount = strchr(header_offset, '\n') - header_offset;
		header_offset += offset_amount;

		// Copy shaders into corresponding strings
		// if(header_offset[ - 8] != '/' && header_offset[ - 7] != '/'){
		// if(strstr(header_offset - (8 + offset_amount) * sizeof(char), "//") == NULL){
			if(strcmp(shader_type_string, "vertex") == 0){
				shader_strings[0] = malloc(shader_end - header_offset);
				memcpy(shader_strings[0], header_offset, shader_end - header_offset - 1);
				shader_strings[0][shader_end - header_offset - 1] = 0; // Null terminate the shader string

			}else if(strcmp(shader_type_string, "fragment") == 0){
				shader_strings[1] = malloc(shader_end - header_offset);
				memcpy(shader_strings[1], header_offset, shader_end - header_offset - 1);
				shader_strings[1][shader_end - header_offset - 1] = 0; // Null terminate the shader string

			}else{
				DebugLog(D_WARN, "Error unknown shader type '%s' specified in file '%s'", shader_type_string, path);
			}
		// }

		// Reset the shader type string
		memset(shader_type_string, 0, sizeof(shader_type_string));
	}while(header_offset != NULL);

    // Tell opengl to compile the shader
	unsigned int shaders[2];



	for(int i = 0; i < 2; i++){
		switch(i){
			case 0:
				shaders[i] = glCreateShader(GL_VERTEX_SHADER);
				break;
			case 1:
				shaders[i] = glCreateShader(GL_FRAGMENT_SHADER);
				break;
		}
		int length = strlen(shader_strings[i]);
		GLCall(glShaderSource(shaders[i], 1, (const GLchar*const*)&shader_strings[i], &length));
		GLCall(glCompileShader(shaders[i]));
		
		// free(shader_strings[i]);

		// Check for errors
		int success;
		GLCall(glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &success));
		if(!success){
			char info_log[1024];
			GLCall(glGetShaderInfoLog(shaders[i], 1024, NULL, info_log));
			DebugLog(D_ERR, "Shader: '%s' compilation failed: %s", path, info_log);
			printf("Shader: '%s' compilation failed: %s", path, info_log);
		}
	}

	// Free variables
    SDL_RWclose(shader_file);
    free(shader_source);
	
	vertex = shaders[0];
	fragment = shaders[1];

	// Compiling the separate shaders into one program
    shader_program.id = glCreateProgram();
    GLCall(glAttachShader(shader_program.id, vertex));
    GLCall(glAttachShader(shader_program.id, fragment));
    GLCall(glLinkProgram(shader_program.id));

    int success;
    GLCall(glGetProgramiv(shader_program.id, GL_LINK_STATUS, &success));
    if(!success){
        char info_log[1024];
        GLCall(glGetProgramInfoLog(shader_program.id, 1024, NULL, info_log));
        DebugLog(D_ERR, "Shader linking failed: %s", info_log);
        printf("Shader linking failed: %s", info_log);
    }

    GLCall(glDeleteShader(vertex));
    GLCall(glDeleteShader(fragment));

	shader_program = ParseShaderUniforms(filename, shader_program.id, shader_strings[0], shader_strings[1]);

	free(path);
	free(shader_strings[0]);
	free(shader_strings[1]);

    return shader_program;
}

void SetShaderProgram(ShaderObject *shader){
	if(current_shader != shader->id){
		glUseProgram(shader->id);
		current_shader = shader->id;
	}
}

void SetVAO(unsigned int vao){
	if(current_vao != vao){
		glBindVertexArray(vao);
		current_vao = vao;
	}
}

int SearchShaderUniformNames(ShaderObject *shader, char *name){
	for(int i = 0; i < shader->num_uniforms; i++){
		if(strcmp(name, shader->uniforms[i].name) == 0){
			return i;
		}
	}
	printf("Couldn't find uniform '%s' in shader: '%s'\n", name, shader->name);
	return -1;
}

void UniformSetBool(ShaderObject *shader, char *uniform_name, bool value){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_BOOL){
    	shader->uniforms[uniform_index]._bool = value;
	}
}

void UniformSetInt(ShaderObject *shader, char *uniform_name, int value){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_INT){
    	shader->uniforms[uniform_index]._int = value;
	}
}

void UniformSetFloat(ShaderObject *shader, char *uniform_name, float value){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_FLOAT){
    	shader->uniforms[uniform_index]._float = value;
	}
}

// Vector (datatype)
void UniformSetVec2(ShaderObject *shader, char *uniform_name, vec2 value){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_VEC2){
    	memcpy(shader->uniforms[uniform_index]._vec2.v, value, sizeof(float) * 2);
	}
}
void UniformSetVec3(ShaderObject *shader, char *uniform_name, vec3 value){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_VEC3){
    	memcpy(shader->uniforms[uniform_index]._vec3.v, value, sizeof(float) * 3);
	}
}
void UniformSetVec4(ShaderObject *shader, char *uniform_name, vec4 value){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_VEC4){
    	memcpy(shader->uniforms[uniform_index]._vec4.v, value, sizeof(float) * 4);
	}
}
// Vector (manual)
void UniformSetVec2_m(ShaderObject *shader, char *uniform_name, float x, float y){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_VEC2){
		shader->uniforms[uniform_index]._vec2.x = x;
		shader->uniforms[uniform_index]._vec2.y = y;
	}
}
void UniformSetVec3_m(ShaderObject *shader, char *uniform_name, float x, float y, float z){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_VEC3){
		shader->uniforms[uniform_index]._vec3.x = x;
		shader->uniforms[uniform_index]._vec3.y = y;
		shader->uniforms[uniform_index]._vec3.z = z;
	}
}
void UniformSetVec4_m(ShaderObject *shader, char *uniform_name, float x, float y, float z, float w){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_VEC4){
		shader->uniforms[uniform_index]._vec4.x = x;
		shader->uniforms[uniform_index]._vec4.y = y;
		shader->uniforms[uniform_index]._vec4.z = z;
		shader->uniforms[uniform_index]._vec4.w = w;
	}
}

// Matrix
void UniformSetMat2(ShaderObject *shader, char *uniform_name, mat2 mat){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_MAT2){
    	memcpy(shader->uniforms[uniform_index]._mat2, mat[0], sizeof(float) * 4);
	}
}
void UniformSetMat3(ShaderObject *shader, char *uniform_name, mat3 mat){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_MAT3){
    	memcpy(shader->uniforms[uniform_index]._mat3, mat[0], sizeof(float) * 9);
	}
}
void UniformSetMat4(ShaderObject *shader, char *uniform_name, mat4 mat){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_MAT4){
    	memcpy(shader->uniforms[uniform_index]._mat4, mat[0], sizeof(float) * 16);
	}
}

// Samplers
void UniformSetSampler1D(ShaderObject *shader, char *uniform_name, int sampler){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_SAMPLER1D){
		shader->uniforms[uniform_index]._sampler1d = sampler;
	}
}
void UniformSetSampler2D(ShaderObject *shader, char *uniform_name, int sampler){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_SAMPLER2D){
		shader->uniforms[uniform_index]._sampler2d = sampler;
	}
}
void UniformSetSampler3D(ShaderObject *shader, char *uniform_name, int sampler){
	int uniform_index = SearchShaderUniformNames(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_SAMPLER3D){
		shader->uniforms[uniform_index]._sampler3d = sampler;
	}
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
		// TODO: Support more image formats / image color formats without crashing
		int internal_format;
		int image_format;
		if(SDL_ISPIXELFORMAT_ALPHA(tmp_surface->format->format)){
			internal_format = GL_RGBA8;
			image_format = GL_RGBA;
		}else{
			internal_format = GL_RGB8;
			image_format = GL_RGB;
		}
		// switch(tmp_surface->format->format){
		// 	case 
		// }
		// // if(format == GL_RGB){
		// 	internal = GL_RGB8;
		// // }
		// printf("name: %s --- format: %s\n", path, SDL_GetPixelFormatName(tmp_surface->format->format));
		// int internal = GL_RGBA8;
		// if(format == GL_RGB){
		// 	internal = GL_RGB8;
		// }
        // GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tmp_surface->w, tmp_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp_surface->pixels));
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, tmp_surface->w, tmp_surface->h, 0, image_format, GL_UNSIGNED_BYTE, tmp_surface->pixels));
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
