#ifndef SCENE_H_
#define SCENE_H_

#include "cgltf.h"
#include "vectorlib.h"
#include "material.h"
#include "gltf.h"


typedef struct Renderer{
	unsigned int vao;

	unsigned int pos_vbo;
	unsigned int norm_vbo;
	unsigned int uv0_vbo;
	unsigned int uv1_vbo;
	unsigned int tan_vbo;

	unsigned int ebo;
}Renderer;

typedef struct Transform{
	Vector3 position;
	Vector3 scale;
	Vector3 rotation_e;
	Vector4 rotation_q;
	mat4 matrix;
}Transform;

typedef struct Model{
	struct Model *parent;
	bool is_loaded;

	unsigned int num_children;
	struct Model *children;

	Transform transform;

	Renderer renderer;

	char *mesh_path;
	unsigned int mesh_index;
	Mesh *mesh;

	char *material_path;
	Material *material;

}Model;

typedef struct Scene{
	char *path;

	unsigned int num_models;
	Model *models;

}Scene;


Model ModelNew(Model *parent, Mesh *mesh, Material *material);
void ModelRender(Model *model);


/**
	=== PLAN: ===
	
	Each model has its own VBO that all of its children share

	VBOs should have 4 pieces of information:
	- Vertex position
	- UV coordinate
	- Normal vector
	- Tangent vector
 */

#endif