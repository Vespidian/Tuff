#ifndef BUNDLE_H_
#define BUNDLE_H_

#include "gltf.h"				// Meshes
#include "textures.h" 			// Textures
#include "shader.h"				// Shaders
#include "material.h" 			// Materials
// #include "" 					// Scripts
// #include "ui/ui_parser.h" 	// Domains
// #include "" 					// Scenes

// typedef struct Model{
// 	char *path;
// 	GLTFState gltf;
// }Model;

typedef struct Bundle{
	char *path;
	bool is_loaded;

	unsigned int num_textures;
	Texture *textures;

	unsigned int num_gltfs;
	GLTF *gltfs;

	unsigned int num_shaders;
	Shader *shaders;

	unsigned int num_materials;
	Material *materials;
}Bundle;

Bundle BundleOpen(char *path);

void BundleFree(Bundle *bundle);

Shader *BundleShaderFind(Bundle *bundle, char *shader_path);
Material *BundleMaterialFind(Bundle *bundle, char *material_path);

#endif