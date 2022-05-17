#include <stdio.h> // TMP
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "json_base.h"
#include "debug.h"

#include "bundle.h"
#include <SDL2/SDL.h> // TMP

static Bundle *bundle_ptr = NULL;;
	static char *dict_textures[] = {
		"path",
		"filtering",
		NULL
	};
	static void tfunc_textures(JSONState *json, unsigned int token){
		if(bundle_ptr != NULL){
			Texture *texture_ptr = &bundle_ptr->textures[bundle_ptr->num_textures];
			if(json->tokens[token].type == JSMN_OBJECT){
				// Allocate space for new texture

				Texture *tmp_textures = realloc(bundle_ptr->textures, sizeof(Texture) * (bundle_ptr->num_textures + 1));
				if(tmp_textures != NULL){
					bundle_ptr->textures = tmp_textures;
					texture_ptr = &bundle_ptr->textures[bundle_ptr->num_textures];

					texture_ptr->gl_tex = 0;
					texture_ptr->is_loaded = false;
					texture_ptr->path = NULL;
					texture_ptr->w = 0;
					texture_ptr->h = 0;

					JSONSetTokenFunc(json, NULL, tfunc_textures);
					JSONParse(json);

					bundle_ptr->num_textures++;
				}
			}else{
				switch(JSONTokenHash(json, token, dict_textures)){
					case 0: // path
						JSONTokenToString(json, token + 1, &texture_ptr->path);
						break;
					case 1: // filtering
						break;
					default:
						break;
				}
			}
		}
	}

	// static char *dict_model[] = {
	// 	"path",
	// 	"",
	// 	NULL
	// };


	static void tfunc_gltfs(JSONState *json, unsigned int token){
		if(bundle_ptr != NULL){
			GLTF *tmp_gltfs = realloc(bundle_ptr->gltfs, sizeof(GLTF) * (bundle_ptr->num_gltfs + 1));
			if(tmp_gltfs != NULL){
				bundle_ptr->gltfs = tmp_gltfs;

				bundle_ptr->gltfs[bundle_ptr->num_gltfs].path = NULL;
				JSONTokenToString(json, token, &bundle_ptr->gltfs[bundle_ptr->num_gltfs].path);

				bundle_ptr->num_gltfs++;
			}
		}
	}

	static void tfunc_shaders(JSONState *json, unsigned int token){
		if(bundle_ptr != NULL){

			Shader *tmp_shaders = realloc(bundle_ptr->shaders, sizeof(Shader) * (bundle_ptr->num_shaders + 1));
			if(tmp_shaders != NULL){
				bundle_ptr->shaders = tmp_shaders;

				// Load the path into the shader's path variable (to be loaded after bundle file is completely read)
				bundle_ptr->shaders[bundle_ptr->num_shaders].path = NULL;
				JSONTokenToString(json, token, &bundle_ptr->shaders[bundle_ptr->num_shaders].path);

				bundle_ptr->num_shaders++;
			}
		}
	}

	static void tfunc_materials(JSONState *json, unsigned int token){
		if(bundle_ptr != NULL){
			Material *tmp_materials = realloc(bundle_ptr->materials, sizeof(Material) * (bundle_ptr->num_materials + 1));
			if(tmp_materials != NULL){
				bundle_ptr->materials = tmp_materials;

				// Load the path into the material's path variable (to be loaded after bundle file is completely read)
				bundle_ptr->materials[bundle_ptr->num_materials].path = NULL;
				JSONTokenToString(json, token, &bundle_ptr->materials[bundle_ptr->num_materials].path);

				bundle_ptr->num_materials++;
			}
		}
	}

static char *dict_bundle[] = {
	"gltfs",
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
			case 0:; // gltfs
				bundle_ptr->gltfs = malloc(sizeof(GLTFState));
				JSONSetTokenFunc(json, NULL, tfunc_gltfs);
				JSONParse(json);
				break;
			case 1:; // textures
				bundle_ptr->textures = malloc(sizeof(Texture));
				JSONSetTokenFunc(json, NULL, tfunc_textures);
				JSONParse(json);
				break;
			case 2:; // shaders
				bundle_ptr->shaders = malloc(sizeof(Shader));
				JSONSetTokenFunc(json, NULL, tfunc_shaders);
				JSONParse(json);
				break;
			case 3:; // materials
				bundle_ptr->materials = malloc(sizeof(Material));
				JSONSetTokenFunc(json, NULL, tfunc_materials);
				JSONParse(json);
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

Bundle BundleNew(){
	Bundle bundle;
	bundle.path = NULL;
	bundle.is_loaded = false;

	bundle.textures = NULL;
	bundle.num_textures = 0;

	bundle.gltfs = NULL;
	bundle.num_gltfs = 0;

	bundle.shaders = NULL;
	bundle.num_shaders = 0;

	bundle.materials = NULL;
	bundle.num_materials = 0;

	return bundle;
}

Bundle BundleOpen(char *path){
	Bundle bundle = BundleNew();

	if(path != NULL){
		bundle.path = malloc(strlen(path) + 1);
		if(bundle.path != NULL){
			memcpy(bundle.path, path, strlen(path));
			bundle.path[strlen(path)] = 0;
		}

		bundle_ptr = &bundle;

		JSONState json = JSONOpen(path);
		if(json.is_loaded){
			JSONSetTokenFunc(&json, NULL, tfunc_bundle);
			JSONParse(&json);
			JSONFree(&json);

			bundle_ptr = NULL;

			// Now we load sub-files

/* --- TEXTURES --- */
			for(int i = 0; i < bundle.num_textures; i++){
				char *path = bundle.textures[i].path;
				if(path != NULL){
					bundle.textures[i].path = NULL;
					bundle.textures[i] = TextureOpen(path);
					free(path);
					path = NULL;
				}
			}

/* --- GLTFS --- */
			for(int i = 0; i < bundle.num_gltfs; i++){
				// int tmp = SDL_GetTicks();
				// bundle.models[i].gltf = GLTFOpen(bundle.models[i].path);
				// if(!bundle.models[i].gltf.is_loaded){
				// 	BundleModelFree(&bundle.models[i]);
				// 	continue;
				// }
				// printf("Took %dms to load gltf file '%s'\n", SDL_GetTicks() - tmp, bundle.models[i].path);
				char *path = bundle.gltfs[i].path;
				if(path != NULL){
					bundle.gltfs[i].path = NULL;
					bundle.gltfs[i] = GLTFOpen(path);
					// BundleMeshCreate()
					free(path);
					path = NULL;
				}
			}

/* --- SHADERS --- */
			for(int i = 0; i < bundle.num_shaders; i++){
				char *path = bundle.shaders[i].path;
				if(path != NULL){
					// Open all the shader files with the previously gathered paths
					bundle.shaders[i].path = NULL;
					bundle.shaders[i] = ShaderOpen(path);
					free(path);
					path = NULL;
				}
			}

/* --- MATERIALS --- */
			for(int i = 0; i < bundle.num_materials; i++){
				char *path = bundle.materials[i].path;


				// TODO: Loop through material's uniforms and set every texture (BundleTextureFind)

				if(path != NULL){
					// Open all the material files with the previously gathered paths
					bundle.materials[i].path = NULL;
					bundle.materials[i] = MaterialOpen(path);

					MaterialSetShader(&bundle.materials[i], BundleShaderFind(&bundle, bundle.materials[i].shader_path));
					MaterialUniformsValidate(&bundle.materials[i]);

					free(path);
					path = NULL;
				}else{
					// TODO: Possibly resize array to get rid of this useless material
				}
			}

			bundle.is_loaded = true;
			DebugLog(D_ACT, "Loaded bundle '%s'", path);
		}
	}
	bundle_ptr = NULL;
	return bundle;
}

/**
 *  TODO: Add flag in Bundles to determine when resources need their references refreshed
 *  This shouldnt be necessary if we stick to using bundles and no seperate variables
 *  Although if we ever need to shift things around in each resource stack this could be useful
 *  Having gaps in the stack could also be an alternative
 */

/**
 * 	Reloading bundle items:
 * 		Create a new instance of that item - ShaderNew()
 * 		Call the load function for that item - ShaderOpen()
 * 		Copy data from instance into the original item
 * 	
 * 	This leaves the pointer address untouched so anything referncing this item still works
 * 
 * 	A ModuleReload() function is to be implemented into every module as appropriate 
 * 	(e.g. ShaderReload())
 */


/*
void BundleReload(Bundle *bundle){
	if(bundle != NULL){
		for(int i = 0; i < bundle->num_textures; i++){
			TextureReload(&bundle->textures[i]);
		}

		for(int i = 0; i < bundle->num_gltfs; i++){
			GLTFReload(&bundle->gltfs[i]);
		}

		for(int i = 0; i < bundle->num_shaders; i++){
			ShaderReload(&bundle->shaders[i]);
		}

		for(int i = 0; i < bundle->num_materials; i++){
			MaterialReload(&bundle->materials[i]);
		}
	}
}
*/

Shader *BundleShaderFind(Bundle *bundle, char *shader_path){
	Shader *shader = NULL;
	if(bundle != NULL && shader_path != NULL){
		for(int i = 0; i < bundle->num_shaders; i++){
			if(strcmp(bundle->shaders[i].path, shader_path) == 0){
				shader = &bundle->shaders[i];
				break;
			}
		}
	}
	return shader;
}

Material *BundleMaterialFind(Bundle *bundle, char *material_path){
	Material *material = NULL;
	if(bundle != NULL && material_path != NULL){
		for(int i = 0; i < bundle->num_materials; i++){
			if(strcmp(bundle->materials[i].path, material_path) == 0){
				material = &bundle->materials[i];
				break;
			}
		}
	}
	return material;
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
		for(int i = 0; i < bundle->num_gltfs; i++){
			GLTFFree(&bundle->gltfs[i]);
		}
		free(bundle->gltfs);
		bundle->gltfs = NULL;
		bundle->num_gltfs = 0;

/* --- SHADERS --- */
		for(int i = 0; i < bundle->num_shaders; i++){
			ShaderFree(&bundle->shaders[i]);
		}
		free(bundle->shaders);
		bundle->shaders = NULL;
		bundle->num_shaders = 0;

/* --- MATERIALS --- */
		for(int i = 0; i < bundle->num_materials; i++){
			MaterialFree(&bundle->materials[i]);
		}
		free(bundle->materials);
		bundle->materials = NULL;
		bundle->num_materials = 0;

	}
}