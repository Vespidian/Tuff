#include <string.h>

#include "debug.h"

#include "json_base.h"
#include "shader.h"

#include "material.h"

Material *material_ptr = NULL;

Material MaterialNew(void){
	Material material;

	material.path = NULL;
	material.name = NULL;
	material.shader_path = NULL;
	material.shader = NULL;

	material.is_validated = false;
	material.is_loaded = false;

	material.num_uniforms = 0;
	material.uniforms = NULL;

	return material;
}

MaterialUniform MaterialUniformNew(){
	MaterialUniform uniform;

	uniform.uniform_name = NULL;
	uniform.texture_path = NULL;
	uniform.value._int = 0;
	uniform.type = UNI_INT;

	return uniform;
}

static char *dict_uniforms[] = {
	"name",
	"value",
	NULL
};

static void tfunc_uniforms(JSONState *json, unsigned int token){
	if(material_ptr != NULL){

		MaterialUniform *uniform_ptr = &material_ptr->uniforms[material_ptr->num_uniforms];
		if(json->tokens[token].type == JSMN_OBJECT){
			
			// Allocate space for new uniform
			MaterialUniform *tmp_uniforms = realloc(material_ptr->uniforms, sizeof(MaterialUniform) * (material_ptr->num_uniforms + 1));
			if(tmp_uniforms != NULL){
				material_ptr->uniforms = tmp_uniforms;
				uniform_ptr = &material_ptr->uniforms[material_ptr->num_uniforms];

				*uniform_ptr = MaterialUniformNew();

				JSONSetTokenFunc(json, NULL, tfunc_uniforms);
				JSONParse(json);

				material_ptr->num_uniforms++;
			}
		}else{
			switch(JSONTokenHash(json, token, dict_uniforms)){
				case 0: // name
					JSONTokenToString(json, token + 1, &uniform_ptr->uniform_name);
					break;
				case 1: // value
					if(uniform_ptr->texture_path == NULL){
						// if(value.type)
						switch(json->tokens[token + 1].type){
							case JSMN_PRIMITIVE:; // bool, int, float
								JSONToken value = JSONTokenValue(json, token + 1);
								switch(value.type){
									case JSON_BOOL:
										uniform_ptr->value._bool = value._bool;
										break;
									case JSON_INT:
										uniform_ptr->value._int = value._int;
										break;
									case JSON_FLOAT:
										uniform_ptr->value._float = value._float;
										break;
									default:
										break;
								}
								break;
							case JSMN_ARRAY:; // vector (bool, int, float)
								// Vectors are maximum 4 elements long
								if(json->tokens[token + 1].size <= 4){
									// Iterate over each element of the vector
									for(int i = 0; i < json->tokens[token + 1].size; i++){
										JSONToken value = JSONTokenValue(json, token + 2 + i);
										switch(value.type){
											case JSON_BOOL:
												uniform_ptr->value._vec4.v[i] = (float)value._bool;
												break;
											case JSON_INT:
												uniform_ptr->value._vec4.v[i] = (float)value._int;
												break;
											case JSON_FLOAT:
												uniform_ptr->value._vec4.v[i] = value._float;
												break;
											default:
												break;
										}
									}
								}
								break;
							case JSMN_STRING:; // texture (string - path to texture)
								JSONTokenToString(json, token + 1, &uniform_ptr->texture_path);
								/** TODO: 
								 * Fix texture module 
								 * Call 'TextureFind' here
								 * Set 'uniform_ptr->value.sampler'
								 */
								break;
							default:
								break;
						}
					}else{
						DebugLog(D_WARN, "%s: Only 1 'value' element allowed per uniform\n", material_ptr->path);
					}
					break;
			}
		}
	}
}

static char *dict_material[] = {
	"name",
	"shader",
	"uniforms",
	NULL
};

static void tfunc_material(JSONState *json, unsigned int token){
	if(material_ptr != NULL){
		switch(JSONTokenHash(json, token, dict_material)){
			case 0: // name
				JSONTokenToString(json, token + 1, &material_ptr->name);
				break;
			case 1: // shader
				JSONTokenToString(json, token + 1, &material_ptr->shader_path);
				break;
			case 2: // uniforms
				material_ptr->uniforms = malloc(sizeof(MaterialUniform));
				JSONSetTokenFunc(json, NULL, tfunc_uniforms);
				JSONParse(json);
				break;
		}
	}
}

void MaterialUniformsValidate(Material *material){
	// Get rid of any uniforms that arent exposed or dont exist in the shader (useless uniforms)
	// ^ OR we could set a boolean which says that this uniform doesnt exist (useful to warn people of mistakes)
	if(material != NULL){
		if(material->shader != NULL){
			// Loop through all the uniforms in the material and delete ones that arent in the parent shader
			for(int i = 0; i < material->num_uniforms; i++){
				ShaderUniform *uniform = ShaderUniformFind(material->shader, material->uniforms[i].uniform_name);
				// If Uniform doesnt exist in the shader OR uniform is not exposed in the shader.. This uniform is useless to us
				// TODO: RETURN: We want uniforms to exist even if they arent defined in .mat files, as long as they exist in a shader, they should also exist in the material (with the exception of matrices)
				if(uniform == NULL){
					DebugLog(D_WARN, "%s: Uniform '%s' does not exist or is not exposed within shader '%s'\n", material->path, material->uniforms[i].uniform_name, material->shader->path);

					MaterialUniform *tmp_uniform = &material->uniforms[i];
					free(tmp_uniform->uniform_name);
					tmp_uniform->uniform_name = NULL;
					free(tmp_uniform->texture_path);
					tmp_uniform->texture_path = NULL;

					if(i + 1 < material->num_uniforms){
						memcpy(&material->uniforms[i], &material->uniforms[i + 1], sizeof(MaterialUniform) * (material->num_uniforms - 1 - i));
					}

					i--;
					material->num_uniforms--;
				}else{
					material->uniforms[i].type = uniform->type;
				}
			}
			// Loop through all the uniforms in the parent shader and copy them to the material
			unsigned int initial_mat_uniforms = material->num_uniforms;
			for(int i = 0; i < material->shader->num_uniforms; i++){
				bool is_defined = false;
				// Check if the uniform is already defined in the material
				for(int k = 0; k < initial_mat_uniforms; k++){
					if(strcmp(material->shader->uniforms[i].name, material->uniforms[k].uniform_name) == 0){
						is_defined = true;
						break;
					}
				}

				// If the uniform isn't defined copy it to the material (as long as its not a matrix)
				UNIFORM_TYPE uniform_type = material->shader->uniforms[i].type;
				if(!is_defined && (uniform_type != UNI_MAT2) && (uniform_type != UNI_MAT3) && (uniform_type != UNI_MAT4)){
				// if(!is_defined && (material->shader->uniforms[i].type < UNI_MAT2) && (material->shader->uniforms[i].type > UNI_MAT4)){
					MaterialUniform *tmp_uniforms = realloc(material->uniforms, sizeof(MaterialUniform) * (material->num_uniforms + 1));
					if(tmp_uniforms != NULL){
						material->uniforms = tmp_uniforms;

						material->uniforms[material->num_uniforms] = MaterialUniformNew();

						char *src_name, *dst_name;
						dst_name = material->uniforms[material->num_uniforms].uniform_name;
						src_name = material->shader->uniforms[i].name;
						dst_name = malloc(strlen(src_name) + 1);
						if(dst_name != NULL){
							memcpy(dst_name, src_name, strlen(src_name));
							dst_name[strlen(src_name)] = 0;
						}

						
						material->uniforms[material->num_uniforms].uniform_name = dst_name;
						material->uniforms[material->num_uniforms].type = material->shader->uniforms[i].type;

						// Set the uniform's value to the shader's default
						MaterialUniform *m_uniform = &material->uniforms[material->num_uniforms];
						ShaderUniform *s_uniform = &material->shader->uniforms[i];
						switch(s_uniform->type){
							case UNI_BOOL:
							case UNI_INT:
							case UNI_SAMPLER1D:
							case UNI_SAMPLER2D:
							case UNI_SAMPLER3D:
								m_uniform->value._int = s_uniform->value_default._int;
								break;
							case UNI_FLOAT:
								m_uniform->value._float = s_uniform->value_default._float;
								break;
							case UNI_VEC4:
								m_uniform->value._vec4.v[3] = s_uniform->value_default._vec4.v[3];
							case UNI_VEC3:
								m_uniform->value._vec4.v[2] = s_uniform->value_default._vec4.v[2];
							case UNI_VEC2:
								m_uniform->value._vec4.v[1] = s_uniform->value_default._vec4.v[1];
								m_uniform->value._vec4.v[0] = s_uniform->value_default._vec4.v[0];
								break;
							default:
								break;
						}

						material->num_uniforms++;
					}
				}
			}
			material->is_validated = true;
		}else{
			DebugLog(D_ERR, "%s: Cannot validate uniforms, shader must first be set using 'MaterialShaderSet'\n", material->path);
		}
	}else{
		DebugLog(D_WARN, "MaterialUniformsValidate: called with NULL argument\n");
	}
}

void MaterialShaderSet(Material *material, Shader *shader){
	if(material != NULL && shader != NULL){
		if(material->shader_path != NULL){
			if(strcmp(material->shader_path, shader->path) == 0){
				material->shader = shader;
			}else{
				DebugLog(D_ERR, "%s: Trying to set material's shader to the incorrect shader (shader paths must match)\n", material->path);
			}
		}else{
			DebugLog(D_ERR, "%s: MaterialShaderSet: shader_path is NULL\n", material->path);
		}
	}else{
		DebugLog(D_WARN, "MaterialShaderSet: called with NULL argument\n");
	}
}

Material MaterialOpen(char *path){
	Material material = MaterialNew();

	if(path != NULL){
		material.path = malloc(strlen(path) + 1);
		memcpy(material.path, path, strlen(path));
		material.path[strlen(path)] = 0;

		material_ptr = &material;

		JSONState json = JSONOpen(path);
		if(json.is_loaded){
			JSONSetTokenFunc(&json, NULL, tfunc_material);
			JSONParse(&json);
			JSONFree(&json);

			if(material.shader_path != NULL){
				/** 
				 * Shader must be set after opening the material using 'MaterialShaderSet' and then 
				 * getting rid of / marking unused uniform with 'MaterialUniformsValidate'
				 */
				material.is_loaded = true;
				DebugLog(D_ACT, "%s: Loaded material", path);

			}else{
				DebugLog(D_ERR, "%s: Material needs parent shader to function", path);
			}
		}
		material_ptr = NULL;
	}

	return material;
}

void MaterialReload(Material *material){
	if(material != NULL){
		char *path = malloc(strlen(material->path) + 1);
		memcpy(path, material->path, strlen(material->path));
		path[strlen(material->path)] = 0;

		Shader *shader = material->shader;

		MaterialFree(material);
		*material = MaterialOpen(path);
		MaterialShaderSet(material, shader);

		free(path);
	}
}

void MaterialFree(Material *material){
	if(material != NULL){
		material->is_loaded = false;

		// Material Path
		free(material->path);
		material->path = NULL;
		
		// Material name
		free(material->name);
		material->name = NULL;

		// Shader path
		free(material->shader_path);
		material->shader_path = NULL;

		// Uniforms
		for(int i = 0; i < material->num_uniforms; i++){
			free(material->uniforms[i].uniform_name);
			material->uniforms[i].uniform_name = NULL;

			free(material->uniforms[i].texture_path);
			material->uniforms[i].texture_path = NULL;
		}
		free(material->uniforms);
		material->uniforms = NULL;
	}
}

MaterialUniform *MaterialUniformFind(Material *material, char *uniform_name){
	MaterialUniform *uniform = NULL;

	if(material != NULL && uniform_name != NULL){
		for(int i = 0; i < material->num_uniforms; i++){
			if(material->uniforms[i].uniform_name != NULL && (strcmp(material->uniforms[i].uniform_name, uniform_name) == 0)){
				uniform = &material->uniforms[i];
				break;
			}
		}
	}

	return uniform;
}

void MaterialShaderPassUniforms(Material *material){
	if(material != NULL && material->shader != NULL && material->shader->is_loaded && material->is_validated){
		// Call 'BundleFindShader' to make sure the path we have is correct
		// ^ Possibly dont do this, and just have a MaterialShaderValidate function we call once every few frames or so for all materials


		// Loop through uniforms and copy them to the shader
		for(int i = 0; i < material->num_uniforms; i++){
			// switch(material->shader->uniforms[i].type){
			// switch(ShaderUniformFind(material->shader, material->uniforms[i].uniform_name)->type){
			switch(material->uniforms[i].type){
				case UNI_BOOL:
					ShaderUniformSetBool(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._bool);
					break;

				case UNI_INT:
					ShaderUniformSetInt(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._int);
					break;

				case UNI_FLOAT:
					ShaderUniformSetFloat(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._float);
					break;

				case UNI_VEC2:
					ShaderUniformSetVec2(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._vec2.v);
					break;
				case UNI_VEC3:
					ShaderUniformSetVec3(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._vec3.v);
					break;
				case UNI_VEC4:
					ShaderUniformSetVec4(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._vec4.v);
					break;
				
				case UNI_SAMPLER1D:
					ShaderUniformSetSampler1D(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._sampler1D);
					break;
				case UNI_SAMPLER2D:
					ShaderUniformSetSampler2D(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._sampler2D);
					break;
				case UNI_SAMPLER3D:
					ShaderUniformSetSampler3D(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._sampler3D);
					break;
				
				default:
					break;
			}
		}

		// Pass shader uniforms to OpenGL
		ShaderPassUniforms(material->shader);
	}
}

//TODO: Add setters and getters for each uniform type (int, float, vec3, texture)

void MaterialUniformSetBool(Material *material, char *uniform_name, bool value){
	if(material != NULL && material->is_validated){
		MaterialUniform *uniform = MaterialUniformFind(material, uniform_name);
		if(uniform != NULL && uniform->type == UNI_BOOL){
			uniform->value._bool = value;
		}
	}
}

void MaterialUniformSetInt(Material *material, char *uniform_name, int value){
	if(material != NULL && material->is_validated){
		MaterialUniform *uniform = MaterialUniformFind(material, uniform_name);
		if(uniform != NULL && uniform->type == UNI_INT){
			uniform->value._int = value;
		}
	}
}

void MaterialUniformSetFloat(Material *material, char *uniform_name, float value){
	if(material != NULL && material->is_validated){
		MaterialUniform *uniform = MaterialUniformFind(material, uniform_name);
		if(uniform != NULL && uniform->type == UNI_FLOAT){
			uniform->value._float = value;
		}
	}
}


void MaterialUniformSetVec2(Material *material, char *uniform_name, vec2 value){
	if(material != NULL && material->is_validated){
		MaterialUniform *uniform = MaterialUniformFind(material, uniform_name);
		if(uniform != NULL && uniform->type == UNI_VEC2){
			memcpy(uniform->value._vec2.v, value, sizeof(float) * 2);
		}
	}
}

void MaterialUniformSetVec3(Material *material, char *uniform_name, vec3 value){
	if(material != NULL && material->is_validated){
		MaterialUniform *uniform = MaterialUniformFind(material, uniform_name);
		if(uniform != NULL && uniform->type == UNI_VEC3){
			memcpy(uniform->value._vec3.v, value, sizeof(float) * 3);
		}
	}
}

void MaterialUniformSetVec4(Material *material, char *uniform_name, vec4 value){
	if(material != NULL && material->is_validated){
		MaterialUniform *uniform = MaterialUniformFind(material, uniform_name);
		if(uniform != NULL && uniform->type == UNI_VEC4){
			memcpy(uniform->value._vec4.v, value, sizeof(float) * 4);
		}
	}
}

// TODO: Make the SetSampler functions use textures in their arguments
void MaterialUniformSetSampler1D(Material *material, char *uniform_name, int sampler){
	if(material != NULL && material->is_validated){
		MaterialUniform *uniform = MaterialUniformFind(material, uniform_name);
		if(uniform != NULL && uniform->type == UNI_SAMPLER1D){
			uniform->value._sampler1D = sampler;
		}
	}
}
void MaterialUniformSetSampler2D(Material *material, char *uniform_name, int sampler){
	if(material != NULL && material->is_validated){
		MaterialUniform *uniform = MaterialUniformFind(material, uniform_name);
		if(uniform != NULL && uniform->type == UNI_SAMPLER2D){
			uniform->value._sampler2D = sampler;
		}
	}
}
void MaterialUniformSetSampler3D(Material *material, char *uniform_name, int sampler){
	if(material != NULL && material->is_validated){
		MaterialUniform *uniform = MaterialUniformFind(material, uniform_name);
		if(uniform != NULL && uniform->type == UNI_SAMPLER3D){
			uniform->value._sampler3D = sampler;
		}
	}
}