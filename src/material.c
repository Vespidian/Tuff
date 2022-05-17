#include <string.h>

#include "debug.h"

#include "json_base.h"
#include "shader.h"
#include "bundle.h"


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

				uniform_ptr->uniform_name = NULL;
				uniform_ptr->texture_path = NULL;
				uniform_ptr->value._int = 0;

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
			for(int i = 0; i < material->num_uniforms; i++){
				int uniform_index = ShaderUniformFind(material->shader, material->uniforms[i].uniform_name);
				// If Uniform doesnt exist in the shader OR material is not exposed in the shader.. This uniform is useless to us
				if(uniform_index == -1 || !material->shader->uniforms[uniform_index].is_exposed){
					DebugLog(D_WARN, "%s: Uniform '%s' does not exist or is not exposed within shader '%s'\n", material->path, material->uniforms[i].uniform_name, material->shader->path);

					MaterialUniform *tmp_uniform = &material->uniforms[i];
					free(tmp_uniform->uniform_name);
					tmp_uniform->uniform_name = NULL;
					free(tmp_uniform->texture_path);
					tmp_uniform->texture_path = NULL;

					memcpy(&material->uniforms[i], &material->uniforms[i + 1], sizeof(MaterialUniform) * (material->num_uniforms - i));

					i--;
					material->num_uniforms--;
				// }else{ // Make sure uniforms conform to shader's 'range' limits
				// 	if(material->uniforms[i].value._float  material->shader->uniforms[uniform_index].min){

				// 	}
				}
			}
			material->is_validated = true;
		}else{
			DebugLog(D_ERR, "%s: Cannot validate uniforms, shader must first be set using 'MaterialSetShader'\n", material->path);
		}
	}
}

void MaterialSetShader(Material *material, Shader *shader){
	if(material != NULL && shader != NULL){
		if(strcmp(material->shader_path, shader->path) == 0){
			material->shader = shader;
		}else{
			DebugLog(D_ERR, "%s: Trying to set material's shader to the incorrect shader (shader paths must match)\n", material->path);
		}
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
				 * Shader must be set after opening the material using 'MaterialSetShader' and then 
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
	}
}

MaterialUniform *MaterialUniformGet(Material *material, char *uniform_name){
	MaterialUniform *uniform = NULL;

	if(material != NULL && uniform_name != NULL){
		for(int i = 0; i < material->num_uniforms; i++){
			if(material->uniforms[i].uniform_name != NULL && (strcmp(material->uniforms[i].uniform_name, uniform_name) == 0)){
				uniform = & material->uniforms[i];
				break;
			}
		}
	}

	return uniform;
}

void MaterialShaderSet(Material *material){
	if(material != NULL && material->shader != NULL && material->shader->is_loaded){
		// Call 'BundleFindShader' to make sure the path we have is correct
		// ^ Possibly dont do this, and just have a MaterialShaderValidate function we call once every few frames or so for all materials


		// Loop through uniforms and copy them to the shader
		for(int i = 0; i < material->num_uniforms; i++){
			// switch(material->shader->uniforms[i].type){
			switch(material->shader->uniforms[ShaderUniformFind(material->shader, material->uniforms[i].uniform_name)].type){
				case UNI_BOOL:
					UniformSetBool(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._bool);
					break;

				case UNI_INT:
					UniformSetInt(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._int);
					break;

				case UNI_FLOAT:
					UniformSetFloat(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._float);
					break;

				case UNI_VEC2:
					UniformSetVec2(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._vec2.v);
					break;
				case UNI_VEC3:
					UniformSetVec3(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._vec3.v);
					break;
				case UNI_VEC4:
					UniformSetVec4(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._vec4.v);
					break;
				
				case UNI_SAMPLER1D:
					UniformSetSampler1D(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._sampler);
					break;
				case UNI_SAMPLER2D:
					UniformSetSampler2D(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._sampler);
					break;
				case UNI_SAMPLER3D:
					UniformSetSampler3D(material->shader, material->uniforms[i].uniform_name, material->uniforms[i].value._sampler);
					break;
				
				default:
					break;
			}
		}

		// Pass shader uniforms to OpenGL
		ShaderPassUniforms(material->shader);
	}
}