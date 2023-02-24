#ifndef BUNDLE_H_
#define BUNDLE_H_

#include "gltf.h"				// Meshes
#include "textures.h" 			// Textures
#include "shader.h"				// Shaders
#include "material.h" 			// Materials
// #include "" 					// Scripts
// #include "ui/ui_parser.h" 	// Domains
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

typedef struct Bundle{
	char *path;
	bool is_loaded;
	bool ptr_addresses_valid;

	unsigned int allocated_textures;
	unsigned int num_textures;
	Texture *textures;

	unsigned int allocated_gltfs;
	unsigned int num_gltfs;
	GLTF *gltfs;

	unsigned int allocated_shaders;
	unsigned int num_shaders;
	Shader *shaders;

	unsigned int allocated_materials;
	unsigned int num_materials;
	Material *materials;
}Bundle;

Bundle BundleOpen(char *path);

void BundleFree(Bundle *bundle);
Bundle BundleNew();

Texture *BundleTextureFind(Bundle *bundle, char *texture_path);
GLTF *BundleGLTFFind(Bundle *bundle, char *gltf_path);
Shader *BundleShaderFind(Bundle *bundle, char *shader_path);
Material *BundleMaterialFind(Bundle *bundle, char *material_path);

#endif