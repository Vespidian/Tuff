#include <stdio.h> // TMP
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "json_base.h"
#include "debug.h"

#include "bundle.h"
#include <SDL2/SDL.h> // TMP

static Bundle *bundle_ptr = NULL;;
	// static Texture ptr_texture;
	// static char *texture_properties_dict[] = {
	// 	"path",
	// 	"filtering",
	// 	"",
	// 	"",
	// 	NULL
	// };
	// static void tfunc_texture_properties(JSONState *json, unsigned int token){
	// 	switch(JSONTokenHash(json, token, texture_properties_dict)){
	// 		case 0:; // path
	// 			JSONToken tmp_token = JSONTokenValue(json, token + 1);
	// 			if(tmp_token.type == JSON_STRING){
	// 				bundle_ptr->textures[bundle_ptr->num_textures] = TextureOpen(tmp_token._string);
	// 			}
	// 			break;
	// 		case 1: // filtering
	// 			break;
	// 		default:
	// 			break;
	// 	}
	// }

	// static void tfunc_textures(JSONState *json, unsigned int token){
	// 	if(json->tokens[token].type == JSMN_STRING){ // "textures" array that holds all other textures
	// 		JSONSetTokenFunc(json, NULL, tfunc_textures);
	// 		JSONParse(json);
	// 	}else if(json->tokens[token].type == JSMN_OBJECT){ // Individual texture
	// 		Texture *tmp_textures = realloc(bundle_ptr->textures, sizeof(Texture) * (bundle_ptr->num_textures + 1));
	// 		if(tmp_textures != NULL){
	// 			bundle_ptr->textures = tmp_textures;
	// 			JSONSetTokenFunc(json, NULL, tfunc_texture_properties);
	// 			JSONParse(json);
	// 			if(bundle_ptr->textures[bundle_ptr->num_textures].is_loaded){ // Only increment the number of textures if the texture actually loaded
	// 				bundle_ptr->num_textures++;
	// 			}
	// 		}else{
	// 			DebugLog(D_WARN, "Could not allocate space for texture");
	// 		}
	// 	}
	// }

	static char *dict_model[] = {
		"path",
		"",
		NULL
	};
	static void tfunc_models(JSONState *json, unsigned int token){
		Model *ptr_models = &bundle_ptr->models[bundle_ptr->num_models];
		if(json->tokens[token].type == JSMN_OBJECT){

			Model *tmp_models = realloc(bundle_ptr->models, sizeof(Model) * (bundle_ptr->num_models + 1));
			if(tmp_models != NULL){
				bundle_ptr->models = tmp_models;

				ptr_models = &bundle_ptr->models[bundle_ptr->num_models];
				ptr_models->path = NULL;
				ptr_models->gltf = GLTFNew();

				JSONSetTokenFunc(json, NULL, tfunc_models);
				JSONParse(json);

				bundle_ptr->num_models++;
			}else{
				bundle_ptr->num_models--;
			}
		}else{
			JSONToken t_value = JSONTokenValue(json, token + 1);
			switch(JSONTokenHash(json, token, dict_model)){
				case 0: // path
					if(t_value.type == JSON_STRING){
						ptr_models->path = NULL;
						JSONTokenToString(json, token + 1, &ptr_models->path);

						// Make sure we didnt already load this same model file
						for(int i = 0; i < bundle_ptr->num_models; i++){
							if(strcmp(ptr_models->path, bundle_ptr->models[i].path) == 0){
								bundle_ptr->num_models--;
								break;
							}
						}

					}else{
						bundle_ptr->num_models--;
					}
					break;
				case 1: // 
					break;
			}
		}
	}

static char *dict_bundle[] = {
	"models",
	"textures",
	"shaders",
	"materials",
	"scripts",
	"domains",
	"scenes",
	NULL
};
static void tfunc_bundle(JSONState *json, unsigned int token){
	if(bundle_ptr != NULL){
		switch(JSONTokenHash(json, token, dict_bundle)){
			case 0:; // models
				bundle_ptr->models = malloc(sizeof(Model));
				JSONSetTokenFunc(json, NULL, tfunc_models);
				JSONParse(json);
				break;
			case 1:; // textures
				break;
			case 2:; // shaders
				break;
			case 3:; // materials
				break;
			case 4:; // scripts
				break;
			case 5:; // domains
				break;
			case 6:; // scenes
				break;
		}
	}
}

void BundleGLTFDelete(Bundle *bundle, unsigned int gltf_index){
	if(bundle != NULL){
		if(gltf_index <= bundle->num_models){
			// Free model's data
			free(bundle->models[gltf_index].path);
			bundle->models[gltf_index].path = NULL;
			GLTFFree(&bundle->models[gltf_index].gltf);

			// Now shift over the rest of the models
			memcpy(&bundle->models[gltf_index], &bundle->models[gltf_index + 1], sizeof(Model) * (bundle->num_models - gltf_index));

			bundle->num_models--;
		}
	}
}

Bundle BundleOpen(char *path){
	Bundle bundle;
	bundle.is_loaded = false;

	bundle.textures = NULL;
	bundle.num_textures = 0;

	bundle.models = NULL;
	bundle.num_models = 0;

	if(path != NULL){
		bundle.path = malloc(strlen(path) + 1);
		if(bundle.path != NULL){
			memcpy(bundle.path, path, strlen(path));
			bundle.path[strlen(path)] = 0;
		}

		bundle_ptr = &bundle;

		JSONState json = JSONOpen(path);
		JSONSetTokenFunc(&json, NULL, tfunc_bundle);
		JSONParse(&json);
		JSONFree(&json);

		bundle_ptr = NULL;

		// Now we load sub-files

		/* --- TEXTURES --- */
		/* --- MODELS --- */
		for(int i = 0; i < bundle.num_models; i++){
			int tmp = SDL_GetTicks();
			bundle.models[i].gltf = GLTFOpen(bundle.models[i].path);
			if(!bundle.models[i].gltf.is_loaded){
				BundleGLTFDelete(&bundle, i);
				i--;
				continue;
			}
			printf("Took %dms to load gltf file '%s'\n", SDL_GetTicks() - tmp, bundle.models[i].path);
		}

		bundle.is_loaded = true;
		DebugLog(D_ACT, "Loaded bundle '%s'", path);
	}
	bundle_ptr = NULL;
	return bundle;
}

void BundleFree(Bundle *bundle){
	if(bundle->is_loaded){
		free(bundle->path);
		bundle->path = NULL;

/* --- TEXTURES --- */
		for(int i = 1; i < bundle->num_textures; i++){
			TextureFree(&bundle->textures[i]);
		}
		free(bundle->textures);
		bundle->textures = NULL;
		bundle->num_textures = 0;

/* --- MODELS --- */
		for(int i = 0; i < bundle->num_models; i++){
			GLTFFree(&bundle->models[i].gltf);
			free(bundle->models[i].path);
			bundle->models[i].path = NULL;
		}
		free(bundle->models);
		bundle->models = NULL;
		bundle->num_models = 0;

	}
}