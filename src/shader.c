#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <GL/glew.h>

#include "vectorlib.h"
#include "debug.h"

#include "json_base.h"
#include "shader.h"


// Prints out everything as it happens
// #define SHADER_DEBUG

const char *shader_types[] = {
	"UNDEFINED",
	"VERTEX",
	"FRAGMENT",
	"GEOMETRY",
	"TESSELATE_CONTROL",
	"TESSELLATE_EVALUATE",
	"COMPUTE",
};

unsigned int current_shader = 0;

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

static ShaderUniform UniformNew(){
	ShaderUniform uniform;
	uniform.name = NULL;
	uniform.description = NULL;
	uniform.uniform = -1;
	uniform.is_exposed = true;
	uniform.has_range = false;
	uniform.is_uploaded = false;
	uniform.type = UNI_FLOAT;
	uniform.min = 0;
	uniform.max = 0;
	uniform.value_default._float = 0;
	uniform.value._float = 0;
	return uniform;
}

void ShaderPassUniforms(Shader *shader){
	if(shader != NULL){
		ShaderSet(shader);
		for(int i = 0; i < shader->num_uniforms; i++){
			if(!shader->uniforms[i].is_uploaded){
				#ifndef NOOPENGL
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
				#endif
				shader->uniforms[i].is_uploaded = true;
			}
		}
	}
}

void ShaderUniformFree(ShaderUniform *uniform){
	if(uniform != NULL){
		free(uniform->name); // LEAK points here: 'name' pointer most likely overwritten
		uniform->name = NULL;
		free(uniform->description);
		uniform->description = NULL;

		uniform->uniform = -1;
		uniform->is_exposed = false;
		uniform->is_uploaded = false;
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
		
			ShaderUniform *tmp_uniforms = realloc(stage_ptr->uniforms, sizeof(ShaderUniform) * (stage_ptr->num_uniforms + 1));
			if(tmp_uniforms != NULL){
				stage_ptr->uniforms = tmp_uniforms;

				stage_ptr->uniforms[stage_ptr->num_uniforms] = UniformNew();

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
					shader->num_texture_slots++;
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
				#ifdef SHADER_DEBUG
					DebugLog(D_ACT, "Got uniform '%s' at source location '%d' in '%s' shader", stage_ptr->uniforms[stage_ptr->num_uniforms].name, source_ptr - stage_ptr->source, stage_ptr->stage_type == STAGE_VERTEX ? "VERTEX" : "FRAGMENT");
				#endif

				// Make sure this uniform doesnt already exist in this stage_ptr
				bool found_copy = false;
				for(int j = 0; j < stage_ptr->num_uniforms; j++){
					if(strcmp(stage_ptr->uniforms[j].name, stage_ptr->uniforms[stage_ptr->num_uniforms].name) == 0){
						found_copy = true;
						// Set the copy's type to the one found in the shader source
						stage_ptr->uniforms[j].type = stage_ptr->uniforms[stage_ptr->num_uniforms].type;
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
	#ifndef NOOPENGL
		shader->id = glCreateProgram();
		for(int i = 0; i < shader->num_stages; i++){
			#ifdef SHADER_DEBUG
				DebugLog(D_ACT, "	%s: stage_id: %d: ", shader->path, i);
			#endif
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
				glShaderSource(shader->stages[i].gl_id, 1, (const GLchar*const*)&shader->stages[i].source, &source_length);
				glCompileShader(shader->stages[i].gl_id);
				int success;
				glGetShaderiv(shader->stages[i].gl_id, GL_COMPILE_STATUS, &success);
				if(!success){
					int info_log_length = 0;
					glGetShaderiv(shader->stages[i].gl_id, GL_INFO_LOG_LENGTH, &info_log_length);
					char *info_log = malloc(info_log_length);
					if(info_log != NULL){
						glGetShaderInfoLog(shader->stages[i].gl_id, info_log_length, NULL, info_log);
						DebugLog(D_ACT, "\n\n'%s' (%s) compilation failed: \n%s\n", shader->path, shader_types[shader->stages[i].stage_type], info_log);
						printf("'%s' (%s) compilation failed: \n%s\n", shader->path, shader_types[shader->stages[i].stage_type], info_log);
					}else{
						DebugLog(D_ERR, "'%s' (%s) could not retrieve info log\n", shader->path, shader_types[shader->stages[i].stage_type]);
					}
					free(info_log);
				}else{
					glAttachShader(shader->id, shader->stages[i].gl_id);
				}
				#ifdef SHADER_DEBUG
					DebugLog(D_ACT, "%s: Compilation successful!\n", shader->path);
				#endif
			}else{
				DebugLog(D_ERR, "%s: Unkown shader stage type\n");
			}
		}
		#ifdef SHADER_DEBUG
			DebugLog(D_ACT, "%s: Done looping stages\n", shader->path);
		#endif

		glLinkProgram(shader->id);
		int success;
		glGetProgramiv(shader->id, GL_LINK_STATUS, &success);
		if(!success){
			int info_log_length = 0;
			glGetProgramiv(shader->id, GL_INFO_LOG_LENGTH, &info_log_length);
			char *info_log = malloc(info_log_length);
			if(info_log != NULL){
				glGetProgramInfoLog(shader->id, info_log_length, NULL, info_log);
				DebugLog(D_ERR, "\n\n'%s' linking failed: \n%s\n", shader->path, info_log);
				printf("'%s' linking failed: \n%s\n", shader->path, info_log);
			}else{
				DebugLog(D_ERR, "'%s' could not retrieve info log\n", shader->path);
			}
			free(info_log);
		}else{
			DebugLog(D_ACT, "%s: Compilation successfull!\n", shader->path);
		}
	#else
		DebugLog(D_ACT, "%s: Shader compilation failed: Since 'NOOPENGL' is defined, no opengl functions will be called\n", shader->path);
	#endif
}

static char *dict_uniform[] = {
	"uniform",
	"description",
	"type",
	"default",
	"range",
	NULL
};

static char *dict_uniform_types[] = {
	"FLOAT",
	"VEC2",
	"VEC3",
	"VEC4",
	"INT",
	"BOOL",
	"SAMPLER1D",
	"SAMPLER2D",
	"SAMPLER3D",
	NULL
};

Shader *shader_ptr = NULL;

	static void tfunc_uniforms(JSONState *json, unsigned int token){
		if(shader_ptr != NULL){
			ShaderStage *stage_ptr = &shader_ptr->stages[shader_ptr->num_stages];
			ShaderUniform *uniform_ptr = &stage_ptr->uniforms[stage_ptr->num_uniforms];
			if(json->tokens[token].type == JSMN_OBJECT){
				// Allocate space for new uniform here

				ShaderUniform *tmp_uniforms = realloc(stage_ptr->uniforms, sizeof(ShaderUniform) * (stage_ptr->num_uniforms + 1));
				if(tmp_uniforms != NULL){
					stage_ptr->uniforms = tmp_uniforms;
					uniform_ptr = &stage_ptr->uniforms[stage_ptr->num_uniforms];

					*uniform_ptr = UniformNew();

					JSONSetTokenFunc(json, NULL, tfunc_uniforms);
					JSONParse(json);

					stage_ptr->num_uniforms++;
				}
			}else{
				switch(JSONTokenHash(json, token, dict_uniform)){
					case 0: // uniform
						// This is the name of the uniform as taken directly from the source
						JSONTokenToString(json, token + 1, &uniform_ptr->name);
						#ifdef SHADER_DEBUG
							DebugLog(D_ACT, "Found uniform: '%s'", uniform_ptr->name);
						#endif
						break;
					case 1: // description
						// Description of the uniform
						uniform_ptr->description = NULL;
						JSONTokenToString(json, token + 1, &uniform_ptr->description);
						#ifdef SHADER_DEBUG
							DebugLog(D_ACT, "Uniform description: '%s'", uniform_ptr->description);
						#endif
						break;
					case 2: // type
						if(JSONTokenValue(json, token + 1).type == JSON_STRING){
							char *string = NULL;
							JSONTokenToString(json, token + 1, &string);

							if(string != NULL){
								switch(JSONTokenHash(json, token + 1, dict_uniform_types)){
									case 0: // FLOAT
										uniform_ptr->type = UNI_FLOAT;
										break;
									case 1: // VEC2
									case 2: // VEC3
									case 3: // VEC4
										uniform_ptr->type = UNI_VEC2 + string[3] - '2';
										break;
									case 4: // INT
										uniform_ptr->type = UNI_INT;
										break;
									case 5: // BOOL
										uniform_ptr->type = UNI_BOOL;
										break;
									case 6: // SAMPLER1D
									case 7: // SAMPLER2D
									case 8: // SAMPLER3D
										// Since all samplers are just 'int' types, we can assign them all together
										uniform_ptr->type = UNI_SAMPLER1D;
										break;
								}
							}

							free(string);
						}
						break;
					case 3: // default
						// Here we have to determine whether the input is one of the following (bool, int, float, vec2, vec3, vec4);
						if(json->tokens[token + 1].type == JSMN_ARRAY){
							// vector
							uniform_ptr->type = UNI_VEC2 + json->tokens[token + 1].size - 2;
							#ifdef SHADER_DEBUG
								DebugLog(D_ACT, "%s: Uniform default value type: '%d'", shader_ptr->path, uniform_ptr->type);
							#endif

							for(int i = 0; i < json->tokens[token + 1].size; i++){
								JSONToken token_value = JSONTokenValue(json, token + 2 + i);
								if(token_value.type == JSON_INT){
									uniform_ptr->value_default._vec4.v[i] = token_value._int;
								}else if(token_value.type == JSON_FLOAT){
									uniform_ptr->value_default._vec4.v[i] = token_value._float;
								}
								#ifdef SHADER_DEBUG
									DebugLog(D_ACT, "%s: Uniform value: '%f'", shader_ptr->path, uniform_ptr->value_default._vec4.v[i]);
								#endif
							}

						}else if(json->tokens[token + 1].type == JSMN_PRIMITIVE){
							// bool, int, float
							JSONToken token_value = JSONTokenValue(json, token + 1);
							switch(token_value.type){
								case JSON_BOOL:
									uniform_ptr->type = UNI_BOOL;
									uniform_ptr->value_default._bool = token_value._bool;
									#ifdef SHADER_DEBUG
										DebugLog(D_ACT, "%s: Uniform value: '%d'bool", shader_ptr->path, uniform_ptr->value_default._bool);
									#endif
									break;
								case JSON_INT:
									uniform_ptr->type = UNI_INT;
									uniform_ptr->value_default._int = token_value._int;
									#ifdef SHADER_DEBUG
										DebugLog(D_ACT, "%s: Uniform value: '%d'int", shader_ptr->path, uniform_ptr->value_default._int);
									#endif
									break;
								case JSON_FLOAT:
									uniform_ptr->type = UNI_FLOAT;
									uniform_ptr->value_default._float = token_value._float;
									#ifdef SHADER_DEBUG
										DebugLog(D_ACT, "%s: Uniform value: '%d'float", uniform_ptr->value_default._float);
									#endif
									break;
								default:
									break;
							}
						}
						break;
					case 4: // range
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
							#ifdef SHADER_DEBUG
								DebugLog(D_ACT, "Min: %f", uniform_ptr->min);
							#endif


							token_value = JSONTokenValue(json, token + 3);
							if(token_value.type == JSON_INT){
								uniform_ptr->max = token_value._int;
							}else if(token_value.type == JSON_FLOAT){
								uniform_ptr->max = token_value._float;
							}else{
								uniform_ptr->max = 0;
							}
							#ifdef SHADER_DEBUG
								DebugLog(D_ACT, "Max: %f", uniform_ptr->max);
							#endif

							uniform_ptr->has_range = true;

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
					#ifdef SHADER_DEBUG
						DebugLog(D_ACT, "Shader name '%s'", shader_ptr->name);
					#endif
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
						#ifdef SHADER_DEBUG
							printf("Stage of type '%d'\n", stage_ptr->stage_type);
						#endif
					}
					break;
				case 2: // uniforms
					stage_ptr->uniforms = malloc(sizeof(ShaderUniform));
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

		#ifdef SHADER_DEBUG
			DebugLog(D_ACT, "Loading shader: '%s'", shader.path);
		#endif
		shader_ptr = &shader;

		JSONState json = JSONOpen(path);
		if(json.is_loaded){
			JSONSetTokenFunc(&json, NULL, tfunc_shader);
			JSONParse(&json);
			JSONFree(&json);
			#ifdef SHADER_DEBUG
				DebugLog(D_ACT, "%s: Loaded json", shader.path);
			#endif

			//Parse internal uniforms from the source code
			for(int i = 0; i < shader.num_stages; i++){
				ShaderStageParseUniforms(&shader, i);
			}
			#ifdef SHADER_DEBUG
				DebugLog(D_ACT, "%s: Parsed uniforms", shader.path);
			#endif
			// Checking for uniforms that are BOTH exposed and internal is done within the 'ShaderStageParseUniforms' function

			// TODO: Possibly come back to this and optimize the checking of redundancies as well as putting uniforms into array

			// Put all uniforms into shader uniform array
			ShaderUniform *tmp_uniforms = NULL;
			for(int i = 0; i < shader.num_stages; i++){
				if(shader.stages[i].num_uniforms > 0){
					tmp_uniforms = realloc(shader.uniforms, sizeof(ShaderUniform) * (shader.num_uniforms + shader.stages[i].num_uniforms + 1));
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
			#ifdef SHADER_DEBUG
				DebugLog(D_ACT, "%s: Copied uniforms", shader.path);
			#endif

			// Check for redundant uniforms
			for(int i = 0; i < shader.num_uniforms; i++){
				for(int j = i + 1; j < shader.num_uniforms; j++){
					if(shader.uniforms[i].name != NULL && shader.uniforms[j].name != NULL){
						if(strcmp(shader.uniforms[i].name, shader.uniforms[j].name) == 0){
							// Found a copy, lets remove it
							#ifdef SHADER_DEBUG
								printf("%s: Removing redundant uniform '%s'", shader.path, shader.uniforms[i].name);
							#endif
							ShaderUniformFree(&shader.uniforms[i]);
							shader.num_uniforms--;
							memmove(&shader.uniforms[i], &shader.uniforms[i + 1], sizeof(ShaderUniform) * (shader.num_uniforms - i));
							i--;
							break;
						}
					}

				}
			}

			// Loop through exposed uniforms and set default values
			for(int i = 0; i < shader.num_uniforms; i++){
				memcpy(&shader.uniforms[i].value, &shader.uniforms[i].value_default, sizeof(shader.uniforms->value_default));
			}

			tmp_uniforms = NULL;
			tmp_uniforms = realloc(shader.uniforms, sizeof(ShaderUniform) * (shader.num_uniforms + 1));
			if(tmp_uniforms != NULL){
				shader.uniforms = tmp_uniforms;

				#ifdef SHADER_DEBUG
					DebugLog(D_ACT, "%s: Removed redundant uniforms", shader.path);
				#endif

				// Compile shader
				ShaderCompile(&shader);

				for(int i = 0; i < shader.num_uniforms; i++){
					#ifndef NOOPENGL
						shader.uniforms[i].uniform = glGetUniformLocation(shader.id, shader.uniforms[i].name);
					#endif
				}

				shader.is_loaded = true;
			}
			#ifdef SHADER_DEBUG
				DebugLog(D_ACT, "%s: Done loading shader", shader.path);
			#endif
		}else{
			DebugLog(D_ERR, "%s: Could not open shader file", path);
		}

		shader_ptr = NULL;
	}
	return shader;
}

void ShaderReload(Shader *shader){
	if(shader != NULL){
		char *path = malloc(strlen(shader->path) + 1);
		memcpy(path, shader->path, strlen(shader->path));
		path[strlen(shader->path)] = 0;

		ShaderFree(shader);
		*shader = ShaderOpen(path);
		free(path);
	}
}

void ShaderFree(Shader *shader){
	if(shader != NULL){
		// Free VRAM data
		#ifndef NOOPENGL
			glDeleteProgram(shader->id);
		#endif

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
			#ifndef NOOPENGL
				glDeleteShader(shader->stages[i].gl_id);
			#endif
		}
		free(shader->stages);
		shader->stages = NULL;

		shader->is_loaded = false;
	}
}

void ShaderSet(Shader *shader){
	if(shader != NULL && current_shader != shader->id){
		#ifndef NOOPENGL
			glUseProgram(shader->id);
		#endif
		current_shader = shader->id;
	}
}

ShaderUniform *ShaderUniformFind(Shader *shader, char *uniform_name){
	ShaderUniform *uniform = NULL;

	if(shader != NULL && uniform_name != NULL){
		for(int i = 0; i < shader->num_uniforms; i++){
			if(shader->uniforms[i].name != NULL && (strcmp(shader->uniforms[i].name, uniform_name) == 0)){
				uniform = &shader->uniforms[i];
				break;
			}
		}
	}

	return uniform;
}

// typedef union ShaderUniformData{
// 	bool _bool;
// 	int _int;
// 	float _float;

// 	Vector2 _vec2;
// 	Vector3 _vec3;
// 	Vector4 _vec4;

// 	mat2 _mat2;
// 	mat3 _mat3;
// 	mat4 _mat4;

// 	int _sampler1d;
// 	int _sampler2d;
// 	int _sampler3d;
// }ShaderUniformData;

// void ShaderUniformSet(Shader *shader, UNIFORM_TYPE type, ShaderUniformData uniform_value){

// }

/* --- PRIMITIVES --- */


void ShaderUniformSetBool(Shader *shader, char *uniform_name, bool value){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_BOOL && (uniform->value._bool != value)){
			uniform->value._bool = value;
			uniform->is_uploaded = false;
		}
	}
}

void ShaderUniformSetInt(Shader *shader, char *uniform_name, int value){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_INT && (uniform->value._int != value)){
			uniform->value._int = value;
			uniform->is_uploaded = false;
		}
	}
}

void ShaderUniformSetFloat(Shader *shader, char *uniform_name, float value){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_FLOAT && (uniform->value._float != value)){
			if(uniform->has_range){
				if(value > uniform->max){
					value = uniform->max;
				}else if (value < uniform->min){
					value = uniform->min;
				}
			}
			uniform->value._float = value;
			uniform->is_uploaded = false;
		}
	}
}

/* --- VECTOR (datatype) --- */

/** TODO: 	Remove the need to call 'ShaderUniformFind' every time we want to set a uniform
 *  		Make all the 'ShaderUniformSet_' functions take a pointer to the uniform or index of uniform rather than the name of the uniform
 */

void ShaderUniformSetVec2(Shader *shader, char *uniform_name, vec2 value){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_VEC2){
			memcpy(uniform->value._vec2.v, value, sizeof(float) * 2);
			uniform->is_uploaded = false;
		}
	}
}
void ShaderUniformSetVec3(Shader *shader, char *uniform_name, vec3 value){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_VEC3){
			memcpy(uniform->value._vec3.v, value, sizeof(float) * 3);
			uniform->is_uploaded = false;
		}
	}
}
void ShaderUniformSetVec4(Shader *shader, char *uniform_name, vec4 value){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_VEC4){
			memcpy(uniform->value._vec4.v, value, sizeof(float) * 4);
			uniform->is_uploaded = false;
		}
	}
}
/* --- VECTOR (manual) --- */


void ShaderUniformSetVec2_m(Shader *shader, char *uniform_name, float x, float y){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_VEC2){
			uniform->value._vec2.x = x;
			uniform->value._vec2.y = y;
			uniform->is_uploaded = false;
		}
	}
}
void ShaderUniformSetVec3_m(Shader *shader, char *uniform_name, float x, float y, float z){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_VEC3){
			uniform->value._vec3.x = x;
			uniform->value._vec3.y = y;
			uniform->value._vec3.z = z;
			uniform->is_uploaded = false;
		}
	}
}
void ShaderUniformSetVec4_m(Shader *shader, char *uniform_name, float x, float y, float z, float w){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_VEC4){
			uniform->value._vec4.x = x;
			uniform->value._vec4.y = y;
			uniform->value._vec4.z = z;
			uniform->value._vec4.w = w;
			uniform->is_uploaded = false;
		}
	}
}

/* --- MATRIX --- */


void ShaderUniformSetMat2(Shader *shader, char *uniform_name, mat2 mat){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_MAT2){
			memcpy(uniform->value._mat2, mat[0], sizeof(float) * 4);
			uniform->is_uploaded = false;
		}
	}
}
void ShaderUniformSetMat3(Shader *shader, char *uniform_name, mat3 mat){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_MAT3){
			memcpy(uniform->value._mat3, mat[0], sizeof(float) * 9);
			uniform->is_uploaded = false;
		}
	}
}
void ShaderUniformSetMat4(Shader *shader, char *uniform_name, mat4 mat){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_MAT4){
			memcpy(uniform->value._mat4, mat[0], sizeof(float) * 16);
			uniform->is_uploaded = false;
		}
	}
}

/* --- SAMPLERS --- */


void ShaderUniformSetSampler1D(Shader *shader, char *uniform_name, int sampler){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_SAMPLER1D && (uniform->value._int != sampler)){
			uniform->value._sampler1d = sampler;
			uniform->is_uploaded = false;
		}
	}
}
void ShaderUniformSetSampler2D(Shader *shader, char *uniform_name, int sampler){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_SAMPLER2D && (uniform->value._int != sampler)){
			uniform->value._sampler2d = sampler;
			uniform->is_uploaded = false;
		}
	}
}
void ShaderUniformSetSampler3D(Shader *shader, char *uniform_name, int sampler){
	if(shader != NULL){
		ShaderUniform *uniform = ShaderUniformFind(shader, uniform_name);
		if(uniform->type == UNI_SAMPLER3D && (uniform->value._int != sampler)){
			uniform->value._sampler3d = sampler;
			uniform->is_uploaded = false;
		}
	}
}
