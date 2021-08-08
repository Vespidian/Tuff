#ifndef MODEL_H_
#define MODEL_H_

#include "../include/cgltf.h"

typedef struct AttributePointer{
	unsigned int size;
	unsigned int type;
	bool normalized;
	size_t stride;
	unsigned int offset;
}AttributePointer;

typedef struct VertexArrayObject{
	unsigned int vao;
	unsigned int num_attribs;
	AttributePointer *attributes;
}VertexArrayObject;

typedef struct MeshObject{
	cgltf_data *data; // Pointer to parsed gltf data
	unsigned int mesh_index; // Index of the mesh in the gltf file that this MeshObject references
	VertexArrayObject vao; // Vertex array object
	unsigned int ebo; // Index buffer object
	unsigned int // All possible attribute types (a vbo for each)
		position_vbo,
		normal_vbo,
		tangent_vbo,
		texcoord_vbo,
		color_vbo,
		joints_vbo,
		weights_vbo;
}MeshObject;


typedef struct MaterialObject{
	unsigned int shader;
	unsigned int textures[16];
	Vector3 color;
}MaterialObject;

typedef struct TransformObject{
	Vector3 position;
	Vector3 scale;
	Vector4 rotation_q;
	Vector3 rotation_e;
	mat4 result;
}TransformObject;

typedef struct ModelObject{
	char *name;
	struct ModelObject *parent;		// Pointer to the parent in the scene heirarchy (NULL if this is a root object)
	struct ModelObject *children;	// Array of all children to this object
	unsigned int num_children;		// Number of direct children this model has
	TransformObject transform;		// Transform for this and all child models
	unsigned int mesh_file;
	unsigned int mesh_index;
	unsigned int shader;
	unsigned int textures[16];
	// -- TODO --
	// physics
}ModelObject;



/**
 *  Possible scene / model ideas:
 *  - have an array of custom scene constants / variables
 */
typedef struct SceneObject{
	char *name;

	cgltf_data **mesh_files;
	unsigned int num_mesh_files;

	MeshObject *meshes;
	unsigned int num_meshes;

	TextureObject *textures;
	unsigned int num_textures;

	ShaderObject *shaders;
	unsigned int num_shaders;

	ModelObject *models;
	unsigned int num_models;

	// -- Scene attributes --
	// skybox
	// physics constants
	// 
}SceneObject;

void InitTransform(TransformObject *transform);
void CalculateTransform(TransformObject *transform);
void CalculateModelTransform(ModelObject *model);

extern SceneObject active_scene;

void InitScene(SceneObject *scene);

ModelObject *NewModel(char *name, ModelObject *parent, TransformObject *transform, unsigned int mesh_file, unsigned int mesh_index, unsigned int shader, unsigned int textures[16]);

#endif