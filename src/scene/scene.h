#ifndef MODEL_H_
#define MODEL_H_

// TODO: This file and its corresponding .c file are to be completely removed

#include "cgltf.h"

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
	// Will be adding more material related parameters later on
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
	// unsigned int shader;
	// unsigned int textures[16];
	unsigned int material;
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

	Texture *textures;
	unsigned int num_textures;

	Shader *shaders;
	unsigned int num_shaders;

	MaterialObject *materials;
	unsigned int num_materials;

	ModelObject *models;
	unsigned int num_models;

	/**
	 *  Scripts
	 */

	// -- Scene attributes --
	// skybox
	// physics constants
	// 
}SceneObject;

// Functions to reload each array within the bundle seperately (Eventually hotloading individual files upon changes)
// Then one function to reload the entire bundle

void InitTransform(TransformObject *transform);
void CalculateTransform(TransformObject *transform);
void CalculateModelTransform(ModelObject *model);

extern SceneObject active_scene;

/**
 *  @brief Initialize an empty scene
 *  @param scene Reference to scene variable to be used. If NULL is specified 'active_scene' is used
 */
void InitScene(SceneObject *scene);

/**
 *  @brief Initialize a ModelObject and return it's pointer
 *  @param name The name of the model
 *  @param parent Model to specify as the new model's parent. If NULL is specified, this object is created at scene root
 *  @param transform Starting transform (translation, rotation, scale). If NULL is specified default / empty transform is used
 *  @param mesh_file Index of raw mesh geometry in scene buffer
 *  @param mesh_index Index of processed mesh geometry in scene buffer
 *  @param material Index of material in scene buffer
 *  @return Pointer to newly created ModelObject
 */
ModelObject *NewModel(char *name, ModelObject *parent, TransformObject *transform, unsigned int mesh_file, unsigned int mesh_index, unsigned int material);

/**
 *  @brief Search for a model in the active scene
 *  @param name Model name of the object to find
 *  @param start Model location / parent to start searching in, NULL specifies scene root
 *  @return Pointer to found ModelObject or NULL if model does not exist
 */
ModelObject *FindModel(char *name, ModelObject *start);

#endif