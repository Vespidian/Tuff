#ifndef BUNDLE_H_
#define BUNDLE_H_

#include "gltf.h"				// Meshes
#include "textures.h" 			// Textures
#include "shader.h"				// Shaders
#include "material.h" 			// Materials
// #include "" 					// Scripts
// #include "ui/ui_parser.h" 	// UI
// #include "" 					// Scenes

/**
 *  Assets to be used in error cases
 */
extern Texture undefined_texture;
extern GLTF undefined_gltf;
extern Mesh undefined_mesh;
extern Shader undefined_shader;
extern Material undefined_material;

void InitUndefined();
void FreeUndefined();


typedef enum BUNDLE_RESOURCES {
	BNDL_TEXTURE, 
	BNDL_GLTF, 
	BNDL_SHADER, 
	BNDL_MATERIAL,
	BNDL_NUM_BUFFS,
} BUNDLE_RESOURCES;

static const size_t BUNDLE_RESOURCE_SIZES[] = {
	sizeof(Texture),
	sizeof(GLTF),
	sizeof(Shader),
	sizeof(Material),
};

typedef struct BundleBuffer{
	union{
		void *anon;
		Texture *textures;
		GLTF * gltfs;
		Shader *shaders;
		Material *materials;
	}data;
	unsigned int size; // # of elements currently stored
	unsigned int capacity; // # of elements that could possibly fit
	BUNDLE_RESOURCES type;
	bool has_been_moved;
}BundleBuffer;

typedef struct Bundle{
	char *path;
	bool is_loaded;

	// The number of buffers should never change once its initialized
	unsigned int num_buffers;
	BundleBuffer *buffers;
}Bundle;


/**
 * Opens the bundle located at 'path' if it exists
 * @param path Pass NULL to 'path' for an empty initialized bundle otherwise it is simply the path to the bundle
 * @returns A struct containing the loaded bundle or an empty bundle if 'path' cannot be opened
*/
Bundle BundleOpen(char *path);

/**
 * Reloads each element of each buffer in 'bundle' from their files
 * ie. textures, shaders, materials, etc.
*/
void BundleReload(Bundle *bundle);

/**
 * Frees the bundle along with all buffers and elements
*/
void BundleFree(Bundle *bundle);


/**
 * Iterates through the textures in 'bundle' until finding a texture with the same path as 'texture_path'
 * @param bundle Bundle to be seraching through
 * @param path Path of texture we want to find
 * @param try_to_open Whether or not the function should attempt to open the path if it doesnt exist in 'bundle'
 * @returns A pointer to the texture within 'bundle'
*/
Texture *BundleTextureFind(Bundle *bundle, char *path, bool try_to_open);

/**
 * Iterates through the GLTFs in 'bundle' until finding a GLTF with the same path as 'gltf_path'
 * @param bundle Bundle to be seraching through
 * @param path Path of GLTF we want to find
 * @param try_to_open Whether or not the function should attempt to open the path if it doesnt exist in 'bundle'
 * @returns A pointer to the GLTF within 'bundle'
*/
GLTF *BundleGLTFFind(Bundle *bundle, char *path, bool try_to_open);

/**
 * Iterates through the shaders in 'bundle' until finding a shader with the same path as 'shader_path'
 * @param bundle Bundle to be seraching through
 * @param path Path of shader we want to find
 * @param try_to_open Whether or not the function should attempt to open the path if it doesnt exist in 'bundle'
 * @returns A pointer to the shader within 'bundle'
*/
Shader *BundleShaderFind(Bundle *bundle, char *path, bool try_to_open);

/**
 * Iterates through the materials in 'bundle' until finding a material with the same path as 'material_path'
 * @param bundle Bundle to be seraching through
 * @param path Path of material we want to find
 * @param try_to_open Whether or not the function should attempt to open the path if it doesnt exist in 'bundle'
 * @returns A pointer to the material within 'bundle'
*/
Material *BundleMaterialFind(Bundle *bundle, char *path, bool try_to_open);


/**
 * Opens the texture and puts it into the bundle's texture buffer
 * @param bundle Bundle to place the texture into
 * @param path Path of texture file
 * @returns A pointer to the texture within the 'bundle'
*/
Texture *BundleTextureOpen(Bundle *bundle, char *path);

/**
 * Opens the GLTF and puts it into the bundle's GLTF buffer
 * @param bundle Bundle to place the GLTF into
 * @param path Path of GLTF file
 * @returns A pointer to the GLTF within the 'bundle'
*/
GLTF *BundleGLTFOpen(Bundle *bundle, char *path);

/**
 * Opens the shader and puts it into the bundle's shader buffer
 * @param bundle Bundle to place the shader into
 * @param path Path of shader file
 * @returns A pointer to the shader within the 'bundle'
*/
Shader *BundleShaderOpen(Bundle *bundle, char *path);

/**
 * Opens the material and puts it into the bundle's material buffer
 * @param bundle Bundle to place the material into
 * @param path Path of material file
 * @returns A pointer to the material within the 'bundle'
*/
Material *BundleMaterialOpen(Bundle *bundle, char *path);


/**
 * Re-assigns pointers for elements who depend on other elements within the bundle
 * Useful when bundle buffer gets moved around because of realloc
 * TODO: Make this automatic by looking at 'buffer.has_been_moved'
 * ie. when using a material, reset its shader ptr if the shader's buffer.has_been_moved is set
*/
void BundleResetPtrs(Bundle *bundle);

#endif