#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <GL/glew.h>
// #include <SDL2/SDL_opengl.h>

#include "vectorlib.h"
#include "debug.h"

#include "json_base.h"
#include "shader.h"

#ifdef DEBUG_BUILD
#define GLCall CheckGLErrors(__FILE__, __LINE__);
#else
#define GLCall
#endif

const char *shader_types[] = {
	"UNDEFINED"
	"VERTEX",
	"FRAGMENT",
	"GEOMETRY",
	"TESSELATE_CONTROL",
	"TESSELLATE_EVALUATE",
	"COMPUTE",
};

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

void ShaderUniformFree(ShaderUniformObject *uniform){
	if(uniform != NULL){
		free(uniform->name); // LEAK points here: 'name' pointer most likely overwritten
		uniform->name = NULL;
		free(uniform->description);
		uniform->description = NULL;

		uniform->uniform = 0;
		uniform->is_exposed = false;
	}
}

// Returns a pointer to the first character that is not 'not_char'
static char *strchr_not(char *str, char not_char){
	while(str[0] != 0 && str[0] == not_char){
		str++;
	}
	return str;
}

static void ShaderStageParseUniforms(Shader *shader, unsigned int stage_id){
	ShaderStage *stage_ptr = &shader->stages[stage_id];
	char *source_ptr = stage_ptr->source;
	if(source_ptr != NULL){
		while((source_ptr = strstr(source_ptr, "uniform ")) != NULL){
			if(strchr(source_ptr, '{') < strchr(source_ptr, ';')){
				source_ptr = strchr(source_ptr, ' '); // Jump the pointer over the 'uniform' keyword
				continue;
			}
		
			ShaderUniformObject *tmp_uniforms = realloc(stage_ptr->uniforms, sizeof(ShaderUniformObject) * (stage_ptr->num_uniforms + 1));
			if(tmp_uniforms != NULL){
				stage_ptr->uniforms = tmp_uniforms;

				stage_ptr->uniforms[stage_ptr->num_uniforms].name = NULL;
				stage_ptr->uniforms[stage_ptr->num_uniforms].description = NULL;
				stage_ptr->uniforms[stage_ptr->num_uniforms].is_exposed = false;
				stage_ptr->uniforms[stage_ptr->num_uniforms].uniform = 0;
				stage_ptr->uniforms[stage_ptr->num_uniforms].value_default._float = 0;
				stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_FLOAT;

				source_ptr = strchr(source_ptr, ' ') + 1;

				if(strncmp(source_ptr, "vec", 3) == 0){
					source_ptr += 3;
					switch((source_ptr)[0]){
						case '2':
							// printf("VEC2\n");
							memset(stage_ptr->uniforms[stage_ptr->num_uniforms].value._vec2.v, 0, sizeof(Vector2));
							stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_VEC2;
							break;
						case '3':
							// printf("VEC3\n");
							memset(stage_ptr->uniforms[stage_ptr->num_uniforms].value._vec3.v, 0, sizeof(Vector3));
							stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_VEC3;
							break;
						case '4':
							// printf("VEC4\n");
							memset(stage_ptr->uniforms[stage_ptr->num_uniforms].value._vec4.v, 0, sizeof(Vector4));
							stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_VEC4;
							break;
					}
					source_ptr += 1;
				}else if(strncmp(source_ptr, "mat", 3) == 0){
					source_ptr += 3;
					switch((source_ptr)[0]){
						case '2':
							// printf("MAT2\n");
							memset(stage_ptr->uniforms[stage_ptr->num_uniforms].value._mat2, 0, sizeof(mat2));
							stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_MAT2;
							break;
						case '3':
							// printf("MAT3\n");
							memset(stage_ptr->uniforms[stage_ptr->num_uniforms].value._mat3, 0, sizeof(mat3));
							stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_MAT3;
							break;
						case '4':
							// printf("MAT4\n");
							memset(stage_ptr->uniforms[stage_ptr->num_uniforms].value._mat4, 0, sizeof(mat4));
							stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_MAT4;
							break;
					}
					source_ptr += 1;
				}else if(strncmp(source_ptr, "sampler", 7) == 0){
					source_ptr += 7;
					switch((source_ptr)[0]){
						case '1':
							stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_SAMPLER1D;
							break;
						case '2':
							stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_SAMPLER2D;
							break;
						case '3':
							stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_SAMPLER3D;
							break;
					}
					source_ptr += 2;
				}else{
					stage_ptr->uniforms[stage_ptr->num_uniforms].value._float = 0;
					if(strncmp(source_ptr, "float", 5) == 0){
						// printf("FLOAT\n");
						source_ptr += 5;
						stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_FLOAT;
					}else if(strncmp(source_ptr, "int", 3) == 0){
						// printf("INT\n");
						source_ptr += 3;
						stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_INT;
					}else if(strncmp(source_ptr, "bool", 4) == 0){
						// printf("BOOL\n");
						source_ptr += 4;
						stage_ptr->uniforms[stage_ptr->num_uniforms].type = UNI_BOOL;
					}
				}
				source_ptr = strchr_not(source_ptr + 1, ' '); // Go to the first character of the uniform's name

				int uniform_name_length = 
					((strchr(source_ptr, ';') > strchr(source_ptr, ' ')) ? strchr(source_ptr, ' ') : strchr(source_ptr, ';'))
					- 
					source_ptr
				;

				// Alloc and copy uniform name
				stage_ptr->uniforms[stage_ptr->num_uniforms].name = malloc(uniform_name_length + 1);
				memcpy(stage_ptr->uniforms[stage_ptr->num_uniforms].name, source_ptr, uniform_name_length);
				stage_ptr->uniforms[stage_ptr->num_uniforms].name[uniform_name_length] = 0;
				DebugLog(D_ACT, "Got uniform '%s' at source location '%d' in '%s' shader", stage_ptr->uniforms[stage_ptr->num_uniforms].name, source_ptr - stage_ptr->source, stage_ptr->stage_type == STAGE_VERTEX ? "VERTEX" : "FRAGMENT");

				// Make sure this uniform doesnt already exist in this stage_ptr
				bool found_copy = false;
				for(int j = 0; j < stage_ptr->num_uniforms; j++){
					if(strcmp(stage_ptr->uniforms[j].name, stage_ptr->uniforms[stage_ptr->num_uniforms].name) == 0){
						found_copy = true;
						break;
					}
				}

				// If this is a copy, remove it
				if(found_copy){
					ShaderUniformFree(&stage_ptr->uniforms[stage_ptr->num_uniforms]);
				}else{
					stage_ptr->num_uniforms++;
				}
			}
		}
	}
}

static void ShaderCompile(Shader *shader){
	shader->id = glCreateProgram();
	for(int i = 0; i < shader->num_stages; i++){
		DebugLog(D_ACT, "	%s: stage_id: %d: ", shader->path, i);
		int shader_type = 0;
		switch(shader->stages[i].stage_type){
			case STAGE_VERTEX:
				shader_type = GL_VERTEX_SHADER;
				break;
			case STAGE_FRAGMENT:
				shader_type = GL_FRAGMENT_SHADER;
				break;
			case STAGE_GEOMETRY:
				shader_type = GL_GEOMETRY_SHADER;
				break;
			case STAGE_TESSELLATE_CONT:
				shader_type = GL_TESS_CONTROL_SHADER;
				break;
			case STAGE_TESSELLATE_EVAL:
				shader_type = GL_TESS_EVALUATION_SHADER;
				break;
			case STAGE_COMPUTE:
				shader_type = GL_COMPUTE_SHADER;
				break;
			case STAGE_UNDEFINED:
				shader_type = 0;
				break;
		}
		if(shader_type != 0){
			shader->stages[i].gl_id = glCreateShader(shader_type);
			int source_length = strlen(shader->stages[i].source);
			GLCall(glShaderSource(shader->stages[i].gl_id, 1, (const GLchar*const*)&shader->stages[i].source, &source_length));
			GLCall(glCompileShader(shader->stages[i].gl_id));
			int success;
			GLCall(glGetShaderiv(shader->stages[i].gl_id, GL_COMPILE_STATUS, &success)); // Crash? 
			if(!success){
				int info_log_length = 0;
				GLCall(glGetShaderiv(shader->stages[i].gl_id, GL_INFO_LOG_LENGTH, &info_log_length));
				char *info_log = malloc(info_log_length);
				if(info_log != NULL){
					GLCall(glGetShaderInfoLog(shader->stages[i].gl_id, info_log_length, NULL, info_log));
					DebugLog(D_ACT, "'%s' (%s) compilation failed: %s", shader->path, shader_types[shader->stages[i].stage_type], info_log);
			DebugLog(D_ACT, "		Got here");
					printf("'%s' (%s) compilation failed: %s", shader->path, shader_types[shader->stages[i].stage_type], info_log);
				}else{
					DebugLog(D_ERR, "'%s' (%s) could not retrieve info log", shader->path, shader_types[shader->stages[i].stage_type]);
				}
				free(info_log);
			}else{
				GLCall(glAttachShader(shader->id, shader->stages[i].gl_id));
			}
			DebugLog(D_ACT, "%s: Compilation successful!", shader->path);
		}else{
			DebugLog(D_ERR, "%s: Unkown shader stage type");
		}
	}
	DebugLog(D_ACT, "%s: Done looping stages", shader->path);

	GLCall(glLinkProgram(shader->id));
	int success;
	GLCall(glGetProgramiv(shader->id, GL_LINK_STATUS, &success));
	if(!success){
		int info_log_length = 0;
		GLCall(glGetProgramiv(shader->id, GL_INFO_LOG_LENGTH, &info_log_length));
		char *info_log = malloc(info_log_length);
		if(info_log != NULL){
			GLCall(glGetProgramInfoLog(shader->id, info_log_length, NULL, info_log));
			DebugLog(D_ERR, "'%s' linking failed: %s", shader->path, info_log);
			printf("'%s' linking failed: %s", shader->path, info_log);
		}else{
			DebugLog(D_ERR, "'%s' could not retrieve info log", shader->path);
		}
		free(info_log);
	}

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
		if(shader_ptr != NULL){
			ShaderStage *stage_ptr = &shader_ptr->stages[shader_ptr->num_stages];
			ShaderUniformObject *uniform_ptr = &stage_ptr->uniforms[stage_ptr->num_uniforms];
			if(json->tokens[token].type == JSMN_OBJECT){
				// Allocate space for new uniform here

				ShaderUniformObject *tmp_uniforms = realloc(stage_ptr->uniforms, sizeof(ShaderUniformObject) * (stage_ptr->num_uniforms + 1));
				if(tmp_uniforms != NULL){
					stage_ptr->uniforms = tmp_uniforms;
					uniform_ptr = &stage_ptr->uniforms[stage_ptr->num_uniforms];

					uniform_ptr->name = NULL;
					uniform_ptr->description = NULL;
					uniform_ptr->uniform = 0;
					uniform_ptr->is_exposed = true;
					uniform_ptr->type = UNI_FLOAT;
					uniform_ptr->min = 0;
					uniform_ptr->max = 0;
					uniform_ptr->value_default._float = 0;
					uniform_ptr->value._float = 0;

					JSONSetTokenFunc(json, NULL, tfunc_uniforms);
					JSONParse(json);

					stage_ptr->num_uniforms++;
				}
			}else{
				switch(JSONTokenHash(json, token, dict_uniform)){
					case 0: // uniform
						// This is the name of the uniform as taken directly from the source
						JSONTokenToString(json, token + 1, &uniform_ptr->name);
						DebugLog(D_ACT, "Found uniform: '%s'", uniform_ptr->name);
						break;
					case 1: // description
						// Description of the uniform
						uniform_ptr->description = NULL;
						JSONTokenToString(json, token + 1, &uniform_ptr->description);
						DebugLog(D_ACT, "Uniform description: '%s'", uniform_ptr->description);
						break;
					case 2: // default
						// Here we have to determine whether the input is one of the following (bool, int, float, vec2, vec3, vec4);
						if(json->tokens[token + 1].type == JSMN_ARRAY){
							// vector
							uniform_ptr->type = UNI_VEC2 + json->tokens[token + 1].size - 2;
							DebugLog(D_ACT, "%s: Uniform default value type: '%d'", shader_ptr->path, uniform_ptr->type);

							for(int i = 0; i < json->tokens[token + 1].size; i++){
								JSONToken token_value = JSONTokenValue(json, token + 2 + i);
								if(token_value.type == JSON_INT){
									uniform_ptr->value_default._vec4.v[i] = token_value._int;
								}else if(token_value.type == JSON_FLOAT){
									uniform_ptr->value_default._vec4.v[i] = token_value._float;
								}
								DebugLog(D_ACT, "%s: Uniform value: '%f'", shader_ptr->path, uniform_ptr->value_default._vec4.v[i]);
							}

						}else if(json->tokens[token + 1].type == JSMN_PRIMITIVE){
							// bool, int, float
							JSONToken token_value = JSONTokenValue(json, token + 1);
							switch(token_value.type){
								case JSON_BOOL:
									uniform_ptr->type = UNI_BOOL;
									uniform_ptr->value_default._bool = token_value._bool;
									DebugLog(D_ACT, "%s: Uniform value: '%d'bool", shader_ptr->path, uniform_ptr->value_default._bool);
									break;
								case JSON_INT:
									uniform_ptr->type = UNI_INT;
									uniform_ptr->value_default._int = token_value._bool;
									DebugLog(D_ACT, "%s: Uniform value: '%d'int", shader_ptr->path, uniform_ptr->value_default._int);
									break;
								case JSON_FLOAT:
									uniform_ptr->type = UNI_FLOAT;
									uniform_ptr->value_default._float = token_value._float;
									DebugLog(D_ACT, "%s: Uniform value: '%d'float", uniform_ptr->value_default._float);
									break;
								default:
									break;
							}
						}
						break;
					case 3: // range
						// Allowed range for the value of the uniform (must be an array of length 2)
						if((json->tokens[token + 1].type = JSMN_ARRAY) && (json->tokens[token + 1].size == 2)){
							// TODO: Possibly add support for integer ranges as well
							JSONToken token_value = JSONTokenValue(json, token + 2);
							if(token_value.type == JSON_INT){
								uniform_ptr->max = token_value._int;
							}else if(token_value.type == JSON_FLOAT){
								uniform_ptr->max = token_value._float;
							}else{
								uniform_ptr->max = 0;
							}
							DebugLog(D_ACT, "Min: %f", uniform_ptr->min);


							token_value = JSONTokenValue(json, token + 3);
							if(token_value.type == JSON_INT){
								uniform_ptr->max = token_value._int;
							}else if(token_value.type == JSON_FLOAT){
								uniform_ptr->max = token_value._float;
							}else{
								uniform_ptr->max = 0;
							}
							DebugLog(D_ACT, "Max: %f", uniform_ptr->max);



						}
						break;
				}
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
			
			ShaderStage *tmp_shader_stages = realloc(shader_ptr->stages, sizeof(ShaderStage) * (shader_ptr->num_stages + 1));
			if(tmp_shader_stages != NULL){
				shader_ptr->stages = tmp_shader_stages;
				stage_ptr = &shader_ptr->stages[shader_ptr->num_stages];

				stage_ptr->gl_id = 0;
				stage_ptr->is_compiled = false;
				stage_ptr->source = NULL;
				stage_ptr->stage_type = STAGE_UNDEFINED;
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
								stage_ptr->stage_type = STAGE_VERTEX;
								break;
							case 1: // FRAGMENT
								stage_ptr->stage_type = STAGE_FRAGMENT;
								break;
							case 2: // GEOMETRY
								stage_ptr->stage_type = STAGE_GEOMETRY;
								break;
							case 3: // TESSELATE_CONTROL
								stage_ptr->stage_type = STAGE_TESSELLATE_CONT;
								break;
							case 4: // TESSELATE_EVALUATE
								stage_ptr->stage_type = STAGE_TESSELLATE_EVAL;
								break;
							case 5: // COMPUTE
								stage_ptr->stage_type = STAGE_COMPUTE;
								break;
							default: // UNDEFINED
								stage_ptr->stage_type = STAGE_UNDEFINED;
								break;
						}
						printf("Stage of type '%d'\n", stage_ptr->stage_type);
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
					break;
			}
		}
	}
}

Shader ShaderOpen(char *path){
	Shader shader = ShaderNew();

	if(path != NULL){
		shader.path = malloc(strlen(path) + 1);
		memcpy(shader.path, path, strlen(path));
		shader.path[strlen(path)] = 0;


		DebugLog(D_ACT, "Loading shader: '%s'", shader.path);
		shader_ptr = &shader;

		JSONState json = JSONOpen(path);
		if(json.is_loaded){
			JSONSetTokenFunc(&json, NULL, tfunc_shader);
			JSONParse(&json);
			JSONFree(&json);
			DebugLog(D_ACT, "%s: Loaded json", shader.path);

			//Parse internal uniforms from the source code
			for(int i = 0; i < shader.num_stages; i++){
				ShaderStageParseUniforms(&shader, i);
			}
			DebugLog(D_ACT, "%s: Parsed uniforms", shader.path);
			// Checking for uniforms that are BOTH exposed and internal is done within the 'ShaderStageParseUniforms' function

			// TODO: Possibly come back to this and optimize the checking of redundancies as well as putting uniforms into array

			// Put all uniforms into shader uniform array
			ShaderUniformObject *tmp_uniforms = NULL;
			for(int i = 0; i < shader.num_stages; i++){
				if(shader.stages[i].num_uniforms != 0){
					tmp_uniforms = realloc(shader.uniforms, sizeof(ShaderUniformObject) * (shader.num_uniforms + shader.stages[i].num_uniforms + 1));
					if(tmp_uniforms != NULL){
						shader.uniforms = tmp_uniforms;
						tmp_uniforms = NULL;

						for(int k = 0; k < shader.stages[i].num_uniforms; k++){
							shader.uniforms[k + shader.num_uniforms] = shader.stages[i].uniforms[k];
						}
						shader.num_uniforms += shader.stages[i].num_uniforms;

					}
				}
			}
			DebugLog(D_ACT, "%s: Copied uniforms", shader.path);

			// Check for redundant uniforms
			for(int i = 0; i < shader.num_uniforms; i++){
				for(int j = i + 1; j < shader.num_uniforms; j++){
					if(shader.uniforms[i].name != NULL && shader.uniforms[j].name != NULL){
						if(strcmp(shader.uniforms[i].name, shader.uniforms[j].name) == 0){
							// Found a copy, lets remove it
							printf("%s: Removing redundant uniform '%s'", shader.path, shader.uniforms[i].name);
							ShaderUniformFree(&shader.uniforms[i]);
							shader.num_uniforms--;
							memmove(&shader.uniforms[i], &shader.uniforms[i + 1], sizeof(ShaderUniformObject) * (shader.num_uniforms - i));
							i--;
							break;
						}
					}

				}
			}

			tmp_uniforms = NULL;
			tmp_uniforms = realloc(shader.uniforms, sizeof(ShaderUniformObject) * (shader.num_uniforms + 1));
			if(tmp_uniforms != NULL){
				shader.uniforms = tmp_uniforms;

				DebugLog(D_ACT, "%s: Removed redundant uniforms", shader.path);

				// Compile each stage

				// Compile shader program
				ShaderCompile(&shader);

				shader.is_loaded = true;
			}
		}

		shader_ptr = NULL;
		DebugLog(D_ACT, "%s: Done loading shader", shader.path);
	}
	return shader;
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
	free(shader->path);
	shader->path = NULL;

	for(int i = 0; i < shader->num_uniforms; i++){
		ShaderUniformFree(&shader->uniforms[i]);
	}
	free(shader->uniforms);
	shader->uniforms = NULL;

	for(int i = 0; i < shader->num_stages; i++){
		free(shader->stages[i].source);
		shader->stages[i].source = NULL;

		free(shader->stages[i].uniforms);
		shader->stages[i].uniforms = NULL;
		GLCall(glDeleteShader(shader->stages[i].gl_id));
	}
	free(shader->stages);
	shader->stages = NULL;

	shader->is_loaded = false;
}

int ShaderSearchUniform(Shader *shader, char *name){
	for(int i = 0; i < shader->num_uniforms; i++){
		if(shader->uniforms[i].name != NULL){
			if(strcmp(name, shader->uniforms[i].name) == 0){
				return i;
			}
		}
	}
	printf("Couldn't find uniform '%s' in shader: '%s'\n", name, shader->name);
	return -1;
}

// --- PRIMITIVES ---


void UniformSetBool(Shader *shader, char *uniform_name, bool value){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_BOOL){
			shader->uniforms[uniform_index].value._bool = value;
		}
	}
}

void UniformSetInt(Shader *shader, char *uniform_name, int value){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_INT){
			shader->uniforms[uniform_index].value._int = value;
		}
	}
}

void UniformSetFloat(Shader *shader, char *uniform_name, float value){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_FLOAT){
			shader->uniforms[uniform_index].value._float = value;
		}
	}
}

// --- VECTOR (datatype) ---


void UniformSetVec2(Shader *shader, char *uniform_name, vec2 value){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_VEC2){
			memcpy(shader->uniforms[uniform_index].value._vec2.v, value, sizeof(float) * 2);
		}
	}
}
void UniformSetVec3(Shader *shader, char *uniform_name, vec3 value){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_VEC3){
			memcpy(shader->uniforms[uniform_index].value._vec3.v, value, sizeof(float) * 3);
		}
	}
}
void UniformSetVec4(Shader *shader, char *uniform_name, vec4 value){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_VEC4){
			memcpy(shader->uniforms[uniform_index].value._vec4.v, value, sizeof(float) * 4);
		}
	}
}
// --- VECTOR (manual) ---


void UniformSetVec2_m(Shader *shader, char *uniform_name, float x, float y){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_VEC2){
			shader->uniforms[uniform_index].value._vec2.x = x;
			shader->uniforms[uniform_index].value._vec2.y = y;
		}
	}
}
void UniformSetVec3_m(Shader *shader, char *uniform_name, float x, float y, float z){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_VEC3){
			shader->uniforms[uniform_index].value._vec3.x = x;
			shader->uniforms[uniform_index].value._vec3.y = y;
			shader->uniforms[uniform_index].value._vec3.z = z;
		}
	}
}
void UniformSetVec4_m(Shader *shader, char *uniform_name, float x, float y, float z, float w){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_VEC4){
			shader->uniforms[uniform_index].value._vec4.x = x;
			shader->uniforms[uniform_index].value._vec4.y = y;
			shader->uniforms[uniform_index].value._vec4.z = z;
			shader->uniforms[uniform_index].value._vec4.w = w;
		}
	}
}

// --- MATRIX ---


void UniformSetMat2(Shader *shader, char *uniform_name, mat2 mat){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_MAT2){
			memcpy(shader->uniforms[uniform_index].value._mat2, mat[0], sizeof(float) * 4);
		}
	}
}
void UniformSetMat3(Shader *shader, char *uniform_name, mat3 mat){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_MAT3){
			memcpy(shader->uniforms[uniform_index].value._mat3, mat[0], sizeof(float) * 9);
		}
	}
}
void UniformSetMat4(Shader *shader, char *uniform_name, mat4 mat){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_MAT4){
			memcpy(shader->uniforms[uniform_index].value._mat4, mat[0], sizeof(float) * 16);
		}
	}
}

// --- SAMPLERS ---


void UniformSetSampler1D(Shader *shader, char *uniform_name, int sampler){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_SAMPLER1D){
			shader->uniforms[uniform_index].value._sampler1d = sampler;
		}
	}
}
void UniformSetSampler2D(Shader *shader, char *uniform_name, int sampler){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_SAMPLER2D){
			shader->uniforms[uniform_index].value._sampler2d = sampler;
		}
	}
}
void UniformSetSampler3D(Shader *shader, char *uniform_name, int sampler){
	int uniform_index = ShaderSearchUniform(shader, uniform_name);
	if(uniform_index > 0){
		if(shader->uniforms[uniform_index].type == UNI_SAMPLER3D){
			shader->uniforms[uniform_index].value._sampler3d = sampler;
		}
	}
}