#ifndef BUNDLE_H_
#define BUNDLE_H_

#include "scene/gltf_loader.h"				// Meshes
#include "textures.h" 		// Textures
// #include "" 				// Shaders
// #include "" 				// Materials
// #include "" 				// Scripts
// #include "ui/ui_parser.h" 	// Domains
// #include "" 				// Scenes

typedef struct Model{
	char *path;
	GLTFState gltf;
}Model;

typedef struct Bundle{
	char *path;
	bool is_loaded;

	unsigned int num_textures;
	Texture *textures;

	unsigned int num_models;
	Model *models;
}Bundle;

Bundle BundleOpen(char *path);

void BundleFree(Bundle *bundle);

#endif