/**
 * Dependency chart
 * 
 * Independent
 * 	Texture
 * 	Shader
 * 	GLTF
 * 
 * Depdendent	-	Depends upon
 * 	Material	-	Shader, Texture (1)
 * 	
 * 
 * (1) - Materials dont yet depend upon textures internally
*/

#include <GL/glew.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "json_read.h"
#include "debug.h"

#include "bundle.h"

Texture undefined_texture;
GLTF undefined_gltf;
Mesh undefined_mesh;
Shader undefined_shader;
Material undefined_material;

/**
 * Utility functions to interact with bundle buffers and manage memory
*/
static BundleBuffer BundleBufferNew();
static bool BundleBufferResize(BundleBuffer *buffer, size_t num_elements);
static bool BundleBufferPush(BundleBuffer *buffer, void *ptr, BUNDLE_RESOURCES type);
static bool BundleBufferFree(BundleBuffer *buffer);

void InitUndefined(){
	undefined_texture = TextureOpen("../bin/builtin_assets/undefined.png", TEXTURE_FILTERING_NEAREST);
	undefined_gltf = GLTFOpen("../bin/builtin_assets/undefined.gltf");
	undefined_mesh = undefined_gltf.meshes[0];
	undefined_shader = ShaderOpen("../bin/builtin_assets/undefined.shader");
	undefined_material = MaterialOpen("../bin/builtin_assets/undefined.mat");
	MaterialShaderSet(&undefined_material, &undefined_shader);
}

void FreeUndefined(){
	TextureFree(&undefined_texture);
	GLTFFree(&undefined_gltf);
	// undefined_mesh = ;
	ShaderFree(&undefined_shader);
	MaterialFree(&undefined_material);
}

/** --- JSON PARSING FUNCTIONS --- **/
static Bundle *bundle_ptr = NULL;
	static char *dict_textures[] = {
		"path",
		"filtering",
		NULL
	};
	static void tfunc_textures(JSONState *json, unsigned int token){
		if(bundle_ptr != NULL){
			// Texture *texture_ptr = &bundle_ptr->buffers[BNDL_TEXTURE].data.textures[bundle_ptr->buffers[BNDL_TEXTURE].size];
			if(json->tokens[token].type != JSMN_OBJECT){
				switch(JSONTokenHash(json, token, dict_textures)){
					case 0:; // path
						char *tmp = NULL;
						JSONTokenToString(json, token + 1, &tmp);
						// texture_ptr = BundleTextureOpen(bundle_ptr, tmp);
						BundleTextureOpen(bundle_ptr, tmp);
						free(tmp);
						break;
					case 1: // filtering
						// texture_ptr; // This is here to avoid variable unused warning (remove)
						break;
					default:
						break;
				}
			}
		}
	}


	static void tfunc_gltfs(JSONState *json, unsigned int token){

		// Load the gltf from the parsed path string
		char *tmp = NULL;
		JSONTokenToString(json, token, &tmp);
		BundleGLTFOpen(bundle_ptr, tmp);
		free(tmp);
	}

	static void tfunc_shaders(JSONState *json, unsigned int token){

		// Load the shader from the parsed path string
		char *tmp = NULL;
		JSONTokenToString(json, token, &tmp);
		BundleShaderOpen(bundle_ptr, tmp);
		free(tmp);
	}

	static void tfunc_materials(JSONState *json, unsigned int token){
		char *tmp = NULL;
		JSONTokenToString(json, token, &tmp);
		BundleMaterialOpen(bundle_ptr, tmp);
		free(tmp);
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
				// bundle_ptr->gltfs = malloc(sizeof(GLTFState));
				BundleBufferResize(&bundle_ptr->buffers[BNDL_GLTF], 1);
				JSONSetTokenFunc(json, NULL, tfunc_gltfs);
				JSONParse(json);
				break;
			case 1:; // textures
				// bundle_ptr->textures = malloc(sizeof(Texture));
				BundleBufferResize(&bundle_ptr->buffers[BNDL_TEXTURE], 1);

				JSONSetTokenFunc(json, NULL, tfunc_textures);
				JSONParse(json);
				break;
			case 2:; // shaders
				// bundle_ptr->shaders = malloc(sizeof(Shader));
				BundleBufferResize(&bundle_ptr->buffers[BNDL_SHADER], 1);

				JSONSetTokenFunc(json, NULL, tfunc_shaders);
				JSONParse(json);
				break;
			case 3:; // materials
				// bundle_ptr->materials = malloc(sizeof(Material));
				BundleBufferResize(&bundle_ptr->buffers[BNDL_MATERIAL], 1);

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

/** --- BUNDLE MEMORY MANAGEMENT FUNCTIONS (private) --- **/

void BundleResetPtrs(Bundle *bundle){
	if(bundle != NULL){
		// Reassign material pointers
		BundleBuffer *materials = &bundle->buffers[BNDL_MATERIAL];

		// Loop through each material and reassign the shader pointers if the shader buffer has been moved
		for(int i = 0; i < materials->size; i++){
			// Shader
			// if(bundle->buffers[BNDL_SHADER].has_been_moved){
				materials->data.materials[i].shader = BundleShaderFind(bundle, materials->data.materials[i].shader_path, false);
			// }
			// bundle->buffers[BNDL_SHADER].has_been_moved = false;

			// Texture
			// - 
		}
	}else{
		DebugLog(D_WARN, "BundleResetPtrs: called with NULL argument\n");
	}
}

/** --- BUFFER UTILITY FUNCTIONS --- **/

static BundleBuffer BundleBufferNew(){
	BundleBuffer buffer;
	buffer.data.anon = NULL;
	buffer.capacity = 0;
	buffer.size = 0;
	buffer.type = BNDL_NUM_BUFFS;
	buffer.has_been_moved = false;
	return buffer;
}

static bool BundleBufferResize(BundleBuffer *buffer, size_t num_elements){
	bool success = false;

	// Make sure the capacity cant be set to less than the number of already present elements
	if((buffer != NULL) && (buffer->size <= num_elements) && (buffer->type != BNDL_NUM_BUFFS)){

		void *tmp = realloc(buffer->data.anon, num_elements * BUNDLE_RESOURCE_SIZES[buffer->type]);
		if(tmp != NULL){
			if(tmp != buffer->data.anon){
				buffer->data.anon = tmp;
				buffer->has_been_moved = true;
			}
			buffer->capacity = num_elements;
			success = true;
		}
	}

	return success;
}

static bool BundleBufferPush(BundleBuffer *buffer, void *ptr, BUNDLE_RESOURCES type){
	bool success = false;

	if((buffer != NULL) && (ptr != NULL) && (buffer->type != BNDL_NUM_BUFFS) && (buffer->type == type)){
		
		// Make sure the buffer has room for another element (attempt to resize if not)
		if(buffer->size + 1 > buffer->capacity){
			
			// If the buffer cannot be resized, return immediately
			if(!BundleBufferResize(buffer, buffer->capacity + 1)){
				return success;
			}
		}

		// Copy the data pointed to by 'ptr' to the correct location in the buffer
		memcpy(buffer->data.anon + (buffer->size * BUNDLE_RESOURCE_SIZES[type]), ptr, BUNDLE_RESOURCE_SIZES[type]);
		buffer->size++;
		success = true;
	}

	return success;
}

static bool BundleBufferReload(Bundle *bundle, unsigned int index){
	bool success = false;

	BundleBuffer *buffer = &bundle->buffers[index];

	if(buffer != NULL){
		// Loop through each element in the buffer and have a switch statement calling a Bundle*Reload
		for(int i = 0; i < buffer->size; i++){
			switch(buffer->type){
				case BNDL_TEXTURE:
					TextureReload(&buffer->data.textures[i]);
					break;
				case BNDL_GLTF:
					GLTFReload(&buffer->data.gltfs[i]);
					break;
				case BNDL_SHADER:
					ShaderReload(&buffer->data.shaders[i]);
					break;
				case BNDL_MATERIAL:
					MaterialReload(&buffer->data.materials[i]);
					MaterialShaderSet(&buffer->data.materials[i], BundleShaderFind(bundle, buffer->data.materials[i].shader_path, true));
					break;
				default:
					break;
			}
		}
		success = true;
	}
	return success;
}

/**
 *  Free the memory of 'buffer'
 *  Any memory allocated by individual elements must be free'd before calling this
*/
static bool BundleBufferFree(BundleBuffer *buffer){
	bool success = false;

	if(buffer != NULL){
		free(buffer->data.anon);
		buffer->type = BNDL_NUM_BUFFS;
		buffer->size = 0;
		buffer->capacity = 0;

		success = true;
	}

	return success;
}

/* Example function to show how we would extract the type buffers like we had before
static Shader *BundleGetShaderBuffer(Bundle *bundle){
	Shader *shader = NULL;
	if(bundle != NULL){
		shader = bundle->buffers[BNDL_SHADER - 1].buffer;
	}
	return shader;
}
*/

// static bool BundleBufferReserve(BundleBuffer *buffer, size_t num_elements){
// 	return BundleBufferResize(buffer, buffer->capacity + num_elements);
// }

void BundleReload(Bundle *bundle){
	if(bundle != NULL){
		for(int i = 0; i < bundle->num_buffers; i++){
			BundleBufferReload(bundle, i);
		}
	}else{
		DebugLog(D_WARN, "BundleReload: called with NULL argument\n");
	}
}

/** --- BUNDLE UTILITY FUNCTIONS --- **/

static Bundle BundleNew(){
	Bundle bundle;
	bundle.path = NULL;
	bundle.is_loaded = false;

	bundle.buffers = malloc(sizeof(BundleBuffer) * BNDL_NUM_BUFFS);
	for(int i = 0; i < BNDL_NUM_BUFFS; i++){
		bundle.buffers[i] = BundleBufferNew();
		bundle.buffers[i].type = i;
	}
	bundle.num_buffers = BNDL_NUM_BUFFS;

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
			// Once the file is in memory we parse it
			JSONSetTokenFunc(&json, NULL, tfunc_bundle);
			JSONParse(&json);
			JSONFree(&json);

			bundle.is_loaded = true;
			DebugLog(D_ACT, "Loaded bundle '%s'", path);
		}
	}else{
		DebugLog(D_WARN, "BundleOpen: called with NULL argument\n");
	}
	bundle_ptr = NULL; // Reset bundle pointer so we cant use it accidentally
	return bundle;
}


/** --- ELEMENT OPEN FUNCTIONS --- **/

Texture *BundleTextureOpen(Bundle *bundle, char *path){
	Texture *texture = &undefined_texture;
	if(bundle != NULL && path != NULL){

		// Search for the resource in the bundle before doing anything
		texture = BundleTextureFind(bundle, path, false);
		
		if(texture != &undefined_texture){
			// If it exists, simply reload it
			TextureReload(texture);

		}else{
			
			// Otherwise, open it
			Texture tmp = TextureOpen(path, TEXTURE_FILTERING_NEAREST);

			// Check that the texture at 'path' exists and then push it to the buffer
			if(tmp.is_loaded){
				if(BundleBufferPush(&bundle->buffers[BNDL_TEXTURE], &tmp, BNDL_TEXTURE)){
					texture = &bundle->buffers[BNDL_TEXTURE].data.textures[bundle->buffers[BNDL_TEXTURE].size - 1];
				}else{
					DebugLog(D_WARN, "BundleTextureOpen: Could not push texture '%s' to bundle '%s'", path, bundle->path);
					TextureFree(&tmp);
				}
			}
		}
	}else{
		DebugLog(D_WARN, "BundleTextureOpen: called with NULL argument\n");
	}
	return texture;
}

GLTF *BundleGLTFOpen(Bundle *bundle, char *path){
	GLTF *gltf = &undefined_gltf;
	if(bundle != NULL && path != NULL){

		// Search for the resource in the bundle before doing anything
		gltf = BundleGLTFFind(bundle, path, false);
		
		if(gltf != &undefined_gltf){
			// If it exists, simply reload it
			GLTFReload(gltf);

		}else{

			// Otherwise, attempt to open the resource
			GLTF tmp = GLTFOpen(path);

			// Check that the gltf at 'path' exists and then push it to the buffer
			if(tmp.gltf_state.is_loaded){
				if(BundleBufferPush(&bundle->buffers[BNDL_GLTF], &tmp, BNDL_GLTF)){
					gltf = &bundle->buffers[BNDL_GLTF].data.gltfs[bundle->buffers[BNDL_GLTF].size - 1];
				}else{
					DebugLog(D_WARN, "BundleGLTFOpen: Could not push gltf '%s' to bundle '%s'", path, bundle->path);
					GLTFFree(&tmp);
				}
			}
		}
	}else{
		DebugLog(D_WARN, "BundleGLTFOpen: called with NULL argument\n");
	}
	return gltf;
}

Shader *BundleShaderOpen(Bundle *bundle, char *path){
	Shader *shader = &undefined_shader;
	if(bundle != NULL && path != NULL){

		// Search for the resource in the bundle before doing anything
		shader = BundleShaderFind(bundle, path, false);
		
		if(shader != &undefined_shader){
			// If it exists, simply reload it
			ShaderReload(shader);

			// Apply the global uniform buffer
			glUniformBlockBinding(shader->id, glGetUniformBlockIndex(shader->id, "ShaderGlobals"), 0);

		}else{

			// Otherwise, attempt to open the resource
			Shader tmp = ShaderOpen(path);

			// Check that the shader at 'path' exists and then push it to the buffer
			if(tmp.is_loaded){
				if(BundleBufferPush(&bundle->buffers[BNDL_SHADER], &tmp, BNDL_SHADER)){
					shader = &bundle->buffers[BNDL_SHADER].data.shaders[bundle->buffers[BNDL_SHADER].size - 1];
					
					// Apply the global uniform buffer
					glUniformBlockBinding(shader->id, glGetUniformBlockIndex(shader->id, "ShaderGlobals"), 0);
				}else{
					DebugLog(D_WARN, "BundleShaderOpen: Could not push shader '%s' to bundle '%s'", path, bundle->path);
					ShaderFree(&tmp);
				}
			}
		}
	}else{
		DebugLog(D_WARN, "BundleShaderOpen: called with NULL argument\n");
	}
	return shader;
}

Material *BundleMaterialOpen(Bundle *bundle, char *path){
	Material *material = &undefined_material;
	if(bundle != NULL && path != NULL){
		
		// Search for the resource in the bundle before doing anything
		material = BundleMaterialFind(bundle, path, false);
		
		if(material != &undefined_material){
			// If it exists, simply reload it
			MaterialReload(material);

		}else{

			// Otherwise, attempt to open the resource
			Material tmp = MaterialOpen(path);

			// Check that the material at 'path' exists and then push it to the buffer
			if(tmp.is_loaded){
				if(BundleBufferPush(&bundle->buffers[BNDL_MATERIAL], &tmp, BNDL_MATERIAL)){
					material = &bundle->buffers[BNDL_MATERIAL].data.materials[bundle->buffers[BNDL_MATERIAL].size - 1];

					MaterialShaderSet(material, BundleShaderFind(bundle, material->shader_path, true));
					MaterialUniformsValidate(material);

					// Iterate through all sampler uniforms and set their values if they have an assigned texture
					for(int k = 0; k < material->num_uniforms; k++){
						if(material->uniforms[k].type == UNI_SAMPLER2D){
							Texture *texture = BundleTextureFind(bundle, material->uniforms[k].texture_path, true);
							if(texture != NULL){
								material->uniforms[k].value._sampler2D = texture->gl_tex;
							}else{
								material->uniforms[k].value._sampler2D = undefined_texture.gl_tex;
							}
						}
					}
				}else{
					DebugLog(D_WARN, "BundleMaterialOpen: Could not push material '%s' to bundle '%s'", path, bundle->path);
					MaterialFree(&tmp);
				}
			}
		}
	}else{
		DebugLog(D_WARN, "BundleMaterialOpen: called with NULL argument\n");
	}
	return material;
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
 * 	A ModuleReload() function is to be implemented into every module as appropriate - THIS HAS BEEN DONE
 * 	(e.g. ShaderReload())
 */

/** --- ELEMENT FIND FUNCTIONS --- **/

Texture *BundleTextureFind(Bundle *bundle, char *path, bool try_to_open){
	Texture *texture = &undefined_texture;
	if(bundle != NULL && path != NULL){

		// Loop through all textures in the bundle and check for match
		for(int i = 0; i < bundle->buffers[BNDL_TEXTURE].size; i++){
			if(bundle->buffers[BNDL_TEXTURE].data.textures[i].path != NULL && strcmp(bundle->buffers[BNDL_TEXTURE].data.textures[i].path, path) == 0){
				texture = &bundle->buffers[BNDL_TEXTURE].data.textures[i];
				break;
			}
		}

		// If a match is not found and 'try_to_open' is true, attempt to open it from file
		if(texture != NULL && texture == &undefined_texture && try_to_open == true){
			texture = BundleTextureOpen(bundle, path);
			DebugLog(D_ACT, "BundleTextureFind: %s: Texture not in bundle, attempting to open from file\n", path);
		}
	}else{
		DebugLog(D_WARN, "BundleTextureFind: %s: called with NULL argument\n", path);
	}
	return texture;
}

GLTF *BundleGLTFFind(Bundle *bundle, char *path, bool try_to_open){
	GLTF *gltf = &undefined_gltf;
	if(bundle != NULL && path != NULL){

		// Loop through all GLTFs in the bundle and check for match
		for(int i = 0; i < bundle->buffers[BNDL_GLTF].size; i++){
			if(bundle->buffers[BNDL_GLTF].data.gltfs[i].path != NULL && strcmp(bundle->buffers[BNDL_GLTF].data.gltfs[i].path, path) == 0){
				gltf = &bundle->buffers[BNDL_GLTF].data.gltfs[i];
				break;
			}
		}

		// If a match is not found and 'try_to_open' is true, attempt to open it from file
		if(gltf != NULL && gltf == &undefined_gltf && try_to_open == true){
			gltf = BundleGLTFOpen(bundle, path);
			DebugLog(D_ACT, "BundleGLTFFind: %s: GLTF not in bundle, attempting to open from file\n", path);
		}
	}else{
		DebugLog(D_WARN, "BundleGLTFFind: %s: called with NULL argument\n", path);
	}
	return gltf;
}

Shader *BundleShaderFind(Bundle *bundle, char *path, bool try_to_open){
	Shader *shader = &undefined_shader;
	if(bundle != NULL && path != NULL){

		// Loop through all shaders in the bundle and check for match
		for(int i = 0; i < bundle->buffers[BNDL_SHADER].size; i++){
			if(bundle->buffers[BNDL_SHADER].data.shaders[i].path != NULL && strcmp(bundle->buffers[BNDL_SHADER].data.shaders[i].path, path) == 0){
				shader = &bundle->buffers[BNDL_SHADER].data.shaders[i];
				break;
			}
		}

		// If a match is not found and 'try_to_open' is true, attempt to open it from file
		if(shader != NULL && shader == &undefined_shader && try_to_open == true){
			shader = BundleShaderOpen(bundle, path);
			DebugLog(D_ACT, "BundleShaderFind: %s: Shader not in bundle, attempting to open from file\n", path);
		}
	}else{
		DebugLog(D_WARN, "BundleShaderFind: %s: called with NULL argument\n", path);
	}
	return shader;
}

Material *BundleMaterialFind(Bundle *bundle, char *path, bool try_to_open){
	Material *material = &undefined_material;
	if(bundle != NULL && path != NULL){

		// Loop through all materials in the bundle and check for match
		for(int i = 0; i < bundle->buffers[BNDL_MATERIAL].size; i++){
			if(bundle->buffers[BNDL_MATERIAL].data.materials[i].path != NULL && strcmp(bundle->buffers[BNDL_MATERIAL].data.materials[i].path, path) == 0){
				material = &bundle->buffers[BNDL_MATERIAL].data.materials[i];
				break;
			}
		}

		// If a match is not found and 'try_to_open' is true, attempt to open it from file
		if(material != NULL && material == &undefined_material && try_to_open == true){
			material = BundleMaterialOpen(bundle, path);
			DebugLog(D_ACT, "BundleShaderFind: %s: Material not in bundle, attempting to open from file\n", path);
		}
	}else{
		DebugLog(D_WARN, "BundleMaterialFind: %s: called with NULL argument\n", path);
	}
	return material;
}

void BundleFree(Bundle *bundle){
	if(bundle != NULL){
		free(bundle->path);
		bundle->path = NULL;

/* --- TEXTURES --- */
		for(int i = 0; i < bundle->buffers[BNDL_TEXTURE].size; i++){
			TextureFree(&bundle->buffers[BNDL_TEXTURE].data.textures[i]);
		}

		if(!BundleBufferFree(&bundle->buffers[BNDL_TEXTURE])){
			DebugLog(D_WARN, "%s: Failed to free texture buffer\n", bundle->path);
		}

/* --- MODELS --- */
		for(int i = 0; i < bundle->buffers[BNDL_GLTF].size; i++){
			GLTFFree(&bundle->buffers[BNDL_GLTF].data.gltfs[i]);
		}

		if(!BundleBufferFree(&bundle->buffers[BNDL_GLTF])){
			DebugLog(D_WARN, "%s: Failed to free GLTF buffer\n", bundle->path);
		}

/* --- SHADERS --- */
		for(int i = 0; i < bundle->buffers[BNDL_SHADER].size; i++){
			ShaderFree(&bundle->buffers[BNDL_SHADER].data.shaders[i]);
		}

		if(!BundleBufferFree(&bundle->buffers[BNDL_SHADER])){
			DebugLog(D_WARN, "%s: Failed to free shader buffer\n", bundle->path);
		}

/* --- MATERIALS --- */
		for(int i = 0; i < bundle->buffers[BNDL_MATERIAL].size; i++){
			MaterialFree(&bundle->buffers[BNDL_MATERIAL].data.materials[i]);
		}

		if(!BundleBufferFree(&bundle->buffers[BNDL_MATERIAL])){
			DebugLog(D_WARN, "%s: Failed to free material buffer\n", bundle->path);
		}


		free(bundle->buffers);
		bundle->num_buffers = 0;

		bundle->is_loaded = false;
	}else{
		DebugLog(D_WARN, "BundleFree: called with NULL argument\n");
	}
}
