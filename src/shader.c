#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <GL/glew.h>
#include <SDL2/SDL.h> // TMP: any SDL_RW calls should be replaced with fopen and fread

#include "vectorlib.h"
#include "debug.h"

#include "json_base.h"
#include "shader.h"

#ifdef DEBUG_BUILD
#define GLCall CheckGLErrors(__FILE__, __LINE__);
#else
#define GLCall
#endif

unsigned int current_shader;

Shader ShaderNew(){
	Shader shader;
	shader.name = NULL;
	shader.is_loaded = false;
	shader.id = 0;
	shader.num_uniforms = 0;
	shader.uniforms = NULL;

	shader.num_stages = 0;
	shader.stages = NULL;

	shader.num_texture_slots = 0;
	return shader;
}

void ShaderPassUniforms(Shader *shader){
	ShaderSet(shader);

	for(int i = 0; i < shader->num_uniforms; i++){
		switch(shader->uniforms[i].type){
			case UNI_BOOL:
				glUniform1i(shader->uniforms[i].uniform, (int)shader->uniforms[i].value._bool);
				break;
			case UNI_INT:
				glUniform1i(shader->uniforms[i].uniform, (int)shader->uniforms[i].value._int);
				break;
			case UNI_FLOAT:
				glUniform1f(shader->uniforms[i].uniform, shader->uniforms[i].value._float);
				break;

			case UNI_VEC2:
				glUniform2fv(shader->uniforms[i].uniform, 1, shader->uniforms[i].value._vec2.v);
				break;
			case UNI_VEC3:
				glUniform3fv(shader->uniforms[i].uniform, 1, shader->uniforms[i].value._vec3.v);
				break;
			case UNI_VEC4:
				glUniform4fv(shader->uniforms[i].uniform, 1, shader->uniforms[i].value._vec4.v);
				break;

			case UNI_MAT2:
				glUniformMatrix2fv(shader->uniforms[i].uniform, 1, GL_FALSE, shader->uniforms[i].value._mat2[0]);
				break;
			case UNI_MAT3:
				glUniformMatrix3fv(shader->uniforms[i].uniform, 1, GL_FALSE, shader->uniforms[i].value._mat3[0]);
				break;
			case UNI_MAT4:
				glUniformMatrix4fv(shader->uniforms[i].uniform, 1, GL_FALSE, shader->uniforms[i].value._mat4[0]);
				break;

			case UNI_SAMPLER1D:
				glUniform1i(shader->uniforms[i].uniform, shader->uniforms[i].value._sampler1d);
				break;
			case UNI_SAMPLER2D:
				glUniform1i(shader->uniforms[i].uniform, shader->uniforms[i].value._sampler2d);
				break;
			case UNI_SAMPLER3D:
				glUniform1i(shader->uniforms[i].uniform, shader->uniforms[i].value._sampler3d);
				break;
		}
	}
}

static Shader ShaderParseUniforms(char *name, unsigned int id, char *vertex, char *fragment){
	Shader shader = ShaderNew();
	
	shader.id = id;
	shader.name = malloc(strlen(name) + 1);
	strcpy(shader.name, name);

	// --- CURRENTLY ONLY SCANS VERTEX STRING ---
	char *uniform_ptr = vertex;
	// int num_samplers = 0;
	for(int i = 0; i < 2; i++){
		while((uniform_ptr = strstr(uniform_ptr, "uniform ")) != NULL){
			if(strchr(uniform_ptr, '{') < strchr(uniform_ptr, ';')){
				uniform_ptr += 8; // Increment by length of "uniform "
				continue;
			}

			ShaderUniformObject *tmp_uniforms = realloc(shader.uniforms, sizeof(ShaderUniformObject) * (shader.num_uniforms + 1));
			if(tmp_uniforms != NULL){
				shader.uniforms = tmp_uniforms;
				uniform_ptr += 8; // Increment by length of "uniform "
				// Make sure this "uniform" keyword isnt referring to a uniform buffer

				// Determine the type of this uniform
				if(strncmp(uniform_ptr, "vec", 3) == 0){
					uniform_ptr += 3;
					switch((uniform_ptr)[0]){
						case '2':
							// printf("VEC2\n");
							memset(shader.uniforms[shader.num_uniforms].value._vec2.v, 0, sizeof(Vector2));
							shader.uniforms[shader.num_uniforms].type = UNI_VEC2;
							break;
						case '3':
							// printf("VEC3\n");
							memset(shader.uniforms[shader.num_uniforms].value._vec3.v, 0, sizeof(Vector2));
							shader.uniforms[shader.num_uniforms].type = UNI_VEC3;
							break;
						case '4':
							// printf("VEC4\n");
							memset(shader.uniforms[shader.num_uniforms].value._vec4.v, 0, sizeof(Vector2));
							shader.uniforms[shader.num_uniforms].type = UNI_VEC4;
							break;
					}
					uniform_ptr += 1;
				}else if(strncmp(uniform_ptr, "mat", 3) == 0){
					uniform_ptr += 3;
					switch((uniform_ptr)[0]){
						case '2':
							// printf("MAT2\n");
							memset(shader.uniforms[shader.num_uniforms].value._mat2, 0, sizeof(mat2));
							shader.uniforms[shader.num_uniforms].type = UNI_MAT2;
							break;
						case '3':
							// printf("MAT3\n");
							memset(shader.uniforms[shader.num_uniforms].value._mat3, 0, sizeof(mat3));
							shader.uniforms[shader.num_uniforms].type = UNI_MAT3;
							break;
						case '4':
							// printf("MAT4\n");
							memset(shader.uniforms[shader.num_uniforms].value._mat4, 0, sizeof(mat4));
							shader.uniforms[shader.num_uniforms].type = UNI_MAT4;
							break;
					}
					uniform_ptr += 1;
				}else if(strncmp(uniform_ptr, "sampler", 7) == 0){
					uniform_ptr += 7;
					shader.num_texture_slots++;
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
					shader.uniforms[shader.num_uniforms].value._float = 0;
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
				shader.uniforms[shader.num_uniforms].name = NULL;
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
					ShaderUniformObject *tmp_uniforms = realloc(shader.uniforms, sizeof(ShaderUniformObject) * (shader.num_uniforms));
					if(tmp_uniforms != NULL){
						shader.uniforms = tmp_uniforms;
					}else{
						DebugLog(D_WARN, "%s: Parsing shader uniforms: realloc could not deallocate space of duplicate uniform", shader.name);
					}
				}else{
					shader.num_uniforms++;
				}
			}else{
				DebugLog(D_WARN, "%s: Parsing shader uniforms: realloc could not allocate space for new shader uniform", shader.name);
			}
		}
		uniform_ptr = fragment; // Do all that over again for the fragment shader
	}
	return shader;
}

static char *dict_uniform[] = {
	"uniform",
	"description",
	"default",
	"range",
	NULL
};

Shader *shader_ptr = NULL;

	static void tfunc_uniforms(JSONState *json, unsigned int token){
		ShaderStage *stage_ptr = &shader_ptr->stages[shader_ptr->num_stages];
		ShaderUniformObject *uniform_ptr = &stage_ptr->uniforms[stage_ptr->num_uniforms];
		if(json->tokens[token].type == JSMN_OBJECT){
			// Allocate space for new uniform here

			ShaderUniformObject *tmp_uniforms = realloc(stage_ptr->uniforms, sizeof(ShaderUniformObject) * (stage_ptr->num_uniforms + 1));
			if(tmp_uniforms != NULL){
				stage_ptr->uniforms = tmp_uniforms;
				uniform_ptr = &stage_ptr->uniforms[stage_ptr->num_uniforms];

				uniform_ptr->uniform = 0;
				uniform_ptr->is_exposed = true;
				uniform_ptr->type = UNI_FLOAT;
				uniform_ptr->min._float = 0;
				uniform_ptr->max._float = 0;
				uniform_ptr->value_default._float = 0;
				uniform_ptr->value._float = 0;

				JSONSetTokenFunc(json, NULL, tfunc_uniforms);
				JSONParse(json);

				stage_ptr->num_uniforms++;
			}
		}else{
			switch(JSONTokenHash(json, token, dict_uniform)){
				// RESUME: Remeber that once shaders and uniforms are parsed, we need to extract uniforms from the source and compare against exposed uniforms to see which to discard
				case 0: // uniform
					// This is the name of the uniform as taken directly from the source
					uniform_ptr->name = NULL;
					JSONTokenToString(json, token + 1, &uniform_ptr->name);
					DebugLog(D_ACT, "Found uniform: '%s'\n", uniform_ptr->name);
					break;
				case 1: // description
					// Description of the uniform

					break;
				case 2: // default
					// Here we have to determine whether the input is one of the following (bool, int, float, vec2, vec3, vec4);

					break;
				case 3: // range
					// Allowed range for the value of the uniform

					break;
			}
		}
	}

static char *dict_shader[] = {
	"name",
	"stage",
	"uniforms",
	"source",
	NULL,
};

static char *dict_stage[] = {
	"VERTEX",
	"FRAGMENT",
	"GEOMETRY",
	"TESSELATE_CONTROL",
	"TESSELATE_EVALUATE",
	"COMPUTE",
	NULL
};

static void tfunc_shader(JSONState *json, unsigned int token){
	ShaderStage *stage_ptr = &shader_ptr->stages[shader_ptr->num_stages];
	if(shader_ptr != NULL){
		if(json->tokens[token].type == JSMN_OBJECT){
			// New shader stage here
			
			ShaderStage *tmp_shader_stages = realloc(shader_ptr->stages, sizeof(ShaderStage) * (shader_ptr->num_uniforms + 1));
			if(tmp_shader_stages != NULL){
				shader_ptr->stages = tmp_shader_stages;
				stage_ptr = &shader_ptr->stages[shader_ptr->num_stages];

				stage_ptr->is_compiled = false;
				stage_ptr->source = NULL;
				stage_ptr->stage = STAGE_UNDEFINED;
				stage_ptr->num_uniforms = 0;
				stage_ptr->uniforms = NULL;

				JSONSetTokenFunc(json, NULL, tfunc_shader);
				JSONParse(json);

				shader_ptr->num_stages++;
			}

		}else{
			switch(JSONTokenHash(json, token, dict_shader)){
				case 0: // name
					shader_ptr->name = NULL;
					JSONTokenToString(json, token + 1, &shader_ptr->name);

					DebugLog(D_ACT, "Shader name '%s'", shader_ptr->name);
					break;
				case 1:; // stage
					JSONToken token_value = JSONTokenValue(json, token + 1);
					if(token_value.type == JSON_STRING){
						switch(JSONTokenHash(json, token + 1, dict_stage)){
							case 0: // VERTEX
								stage_ptr->stage = STAGE_VERTEX;
								break;
							case 1: // FRAGMENT
								stage_ptr->stage = STAGE_FRAGMENT;
								break;
							case 2: // GEOMETRY
								stage_ptr->stage = STAGE_GEOMETRY;
								break;
							case 3: // TESSELATE_CONTROL
								stage_ptr->stage = STAGE_TESSELATE_CONT;
								break;
							case 4: // TESSELATE_EVALUATE
								stage_ptr->stage = STAGE_TESSELATE_EVAL;
								break;
							case 5: // COMPUTE
								stage_ptr->stage = STAGE_COMPUTE;
								break;
							default: // UNDEFINED
								stage_ptr->stage = STAGE_UNDEFINED;
								break;
						}
						printf("Stage of type '%s':'%d'\n", token_value._string, stage_ptr->stage);
					}
					break;
				case 2: // uniforms
					stage_ptr->uniforms = malloc(sizeof(ShaderUniformObject));
					JSONSetTokenFunc(json, NULL, tfunc_uniforms);
					JSONParse(json);
					break;
				case 3: // source
					stage_ptr->source = NULL;
					JSONTokenToString(json, token + 1, &stage_ptr->source);
					// printf("\n\n----- Found shader source: -----\n%s\n\n", stage_ptr->source);
					DebugLog(D_ACT,"\n\n----- Found shader source: -----\n%s\n\n", stage_ptr->source);
					break;
			}
		}
	}
}

Shader ShaderOpen_new(char *path){
	Shader shader = ShaderNew();

	if(path != NULL){
		int t = SDL_GetTicks();
		shader.path = malloc(strlen(path) + 1);
		memcpy(shader.path, path, strlen(path));
		shader.path[strlen(path)] = 0;

		shader_ptr = &shader;

		JSONState json = JSONOpen(path);
		if(json.is_loaded){
			JSONSetTokenFunc(&json, NULL, tfunc_shader);
			JSONParse(&json);
			JSONFree(&json);


		}

		shader_ptr = NULL;

		printf("\nLoading shader '%s' took %dms\n", path, SDL_GetTicks() - t);
	}
	return shader;
}

Shader ShaderOpen(char *filename){ // TODO: This whole function must be reworked to support the new json shader format
    SDL_RWops *shader_file;
    char *shader_source = NULL;
    int file_size;

	Shader shader_program;
	shader_program.is_loaded = false;
    unsigned int vertex, fragment;

    // Read shader source from file
	char *path = NULL;
	path = malloc(strlen(filename) + 1);
	memcpy(path, filename, strlen(filename));
	path[strlen(filename)] = 0;
	
    shader_file = SDL_RWFromFile(path, "rb");
    if(shader_file == NULL){
        DebugLog(D_ERR, "Shader loading error! Shader file: '%s' not found", path);
        printf("Shader loading error! Shader file: '%s' not found", path);
		// SHOULD EXIT THE FUNCTION HERE
		free(path);
		path = NULL;
		return shader_program;
    }

    // Find length of shader source file
    file_size = SDL_RWseek(shader_file, 0, SEEK_END);
    SDL_RWseek(shader_file, 0, SEEK_SET);

    // Allocate space for shader string
    shader_source = malloc(file_size + 2);

    // Read shader to string and null terminate it
    SDL_RWread(shader_file, shader_source, 1, file_size);
    shader_source[file_size + 1] = '\0';

	// Split string into seperate strings for each shader type
	char *shader_strings[2];
	shader_strings[0] = NULL;
	shader_strings[1] = NULL;
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
				shader_strings[0] = malloc(shader_end - header_offset + 1);
				memcpy(shader_strings[0], header_offset, shader_end - header_offset - 1);
				shader_strings[0][shader_end - header_offset - 1] = 0; // Null terminate the shader string

			}else if(strcmp(shader_type_string, "fragment") == 0){
				shader_strings[1] = malloc(shader_end - header_offset + 1);
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

	shader_program = ShaderParseUniforms(filename, shader_program.id, shader_strings[0], shader_strings[1]);

	free(path);
	free(shader_strings[0]);
	free(shader_strings[1]);
	shader_program.is_loaded = true;

    return shader_program;
}

void ShaderSet(Shader *shader){
	if(current_shader != shader->id){
		glUseProgram(shader->id);
		current_shader = shader->id;
	}
}

void ShaderFree(Shader *shader){
	// Free VRAM data
	GLCall(glDeleteProgram(shader->id));

	// Free RAM data
	free(shader->name);
	shader->name = NULL;

	for(int i = 0; i < shader->num_uniforms; i++){
		free(shader->uniforms[i].name);
		shader->uniforms[i].name = NULL;
	}
	free(shader->uniforms);
	shader->uniforms = NULL;

	shader->is_loaded = false;
}

int ShaderSearchUniform(Shader *shader, char *name){
	for(int i = 0; i < shader->num_uniforms; i++){
		if(strcmp(name, shader->uniforms[i].name) == 0){
			return i;
		}
	}
	printf("Couldn't find uniform '%s' in shader: '%s'\n", name, shader->name);
	return -1;
}

void UniformSetBool(Shader *shader, char *uniform_name, bool value){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_BOOL){
    	shader->uniforms[uniform_index].value._bool = value;
	}
}

void UniformSetInt(Shader *shader, char *uniform_name, int value){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_INT){
    	shader->uniforms[uniform_index].value._int = value;
	}
}

void UniformSetFloat(Shader *shader, char *uniform_name, float value){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_FLOAT){
    	shader->uniforms[uniform_index].value._float = value;
	}
}

// Vector (datatype)
void UniformSetVec2(Shader *shader, char *uniform_name, vec2 value){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_VEC2){
    	memcpy(shader->uniforms[uniform_index].value._vec2.v, value, sizeof(float) * 2);
	}
}
void UniformSetVec3(Shader *shader, char *uniform_name, vec3 value){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_VEC3){
    	memcpy(shader->uniforms[uniform_index].value._vec3.v, value, sizeof(float) * 3);
	}
}
void UniformSetVec4(Shader *shader, char *uniform_name, vec4 value){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_VEC4){
    	memcpy(shader->uniforms[uniform_index].value._vec4.v, value, sizeof(float) * 4);
	}
}
// Vector (manual)
void UniformSetVec2_m(Shader *shader, char *uniform_name, float x, float y){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_VEC2){
		shader->uniforms[uniform_index].value._vec2.x = x;
		shader->uniforms[uniform_index].value._vec2.y = y;
	}
}
void UniformSetVec3_m(Shader *shader, char *uniform_name, float x, float y, float z){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_VEC3){
		shader->uniforms[uniform_index].value._vec3.x = x;
		shader->uniforms[uniform_index].value._vec3.y = y;
		shader->uniforms[uniform_index].value._vec3.z = z;
	}
}
void UniformSetVec4_m(Shader *shader, char *uniform_name, float x, float y, float z, float w){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_VEC4){
		shader->uniforms[uniform_index].value._vec4.x = x;
		shader->uniforms[uniform_index].value._vec4.y = y;
		shader->uniforms[uniform_index].value._vec4.z = z;
		shader->uniforms[uniform_index].value._vec4.w = w;
	}
}

// Matrix
void UniformSetMat2(Shader *shader, char *uniform_name, mat2 mat){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_MAT2){
    	memcpy(shader->uniforms[uniform_index].value._mat2, mat[0], sizeof(float) * 4);
	}
}
void UniformSetMat3(Shader *shader, char *uniform_name, mat3 mat){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_MAT3){
    	memcpy(shader->uniforms[uniform_index].value._mat3, mat[0], sizeof(float) * 9);
	}
}
void UniformSetMat4(Shader *shader, char *uniform_name, mat4 mat){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_MAT4){
    	memcpy(shader->uniforms[uniform_index].value._mat4, mat[0], sizeof(float) * 16);
	}
}

// Samplers
void UniformSetSampler1D(Shader *shader, char *uniform_name, int sampler){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_SAMPLER1D){
		shader->uniforms[uniform_index].value._sampler1d = sampler;
	}
}
void UniformSetSampler2D(Shader *shader, char *uniform_name, int sampler){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_SAMPLER2D){
		shader->uniforms[uniform_index].value._sampler2d = sampler;
	}
}
void UniformSetSampler3D(Shader *shader, char *uniform_name, int sampler){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(shader->uniforms[uniform_index].type == UNI_SAMPLER3D){
		shader->uniforms[uniform_index].value._sampler3d = sampler;
	}
}