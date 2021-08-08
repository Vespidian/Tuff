#include "../global.h"

#include "scene.h"

SceneObject active_scene;

void InitTransform(TransformObject *transform){
	transform->position = (Vector3){0};
	transform->rotation_e = (Vector3){0};
	transform->rotation_q = (Vector4){0};
	transform->scale = (Vector3){1, 1, 1};
	glm_mat4_identity(transform->result);
}

void CalculateTransform(TransformObject *transform){
	// translate, rotate, scale
	mat4 translation;
	mat4 rotation;
	mat4 scale;

	glm_mat4_identity(translation);
	glm_mat4_identity(rotation);
	glm_mat4_identity(scale);
	glm_mat4_identity(transform->result);

	glm_translate(translation, transform->position.v);
	glm_euler(transform->rotation_e.v, rotation);
	glm_scale(scale, transform->scale.v);

	glm_mul(translation, rotation, transform->result);
	glm_mul(transform->result, scale, transform->result);
}

void UpdateTransformChildren(ModelObject *model, mat4 transform){
	// Add the parent's transform
	glm_mul(model->transform.result, transform, model->transform.result);

	// Loop through children and add this model's transform to them
	for(int i = 0; i < model->num_children; i++){
		UpdateTransformChildren(&model->children[i], model->transform.result);
	}
}

void CalculateModelTransform(ModelObject *model){
	ModelObject *parent = model;

	// Calculate this model's raw transformation
	CalculateTransform(&model->transform);

	// Add all parent transformations to this model's transformation matrix
	while((parent = parent->parent) != NULL){
		CalculateTransform(&parent->transform);
		glm_mul(model->transform.result, parent->transform.result, model->transform.result);
	}

	// Recursively applies this model's transform to every child
	UpdateTransformChildren(model, model->transform.result);
}

ModelObject *NewModel(char *name, ModelObject *parent, TransformObject *transform, unsigned int mesh_file, unsigned int mesh_index, unsigned int shader, unsigned int textures[16]){
	// Model
	ModelObject *model;
	if(parent == NULL){
		active_scene.models = realloc(active_scene.models, sizeof(ModelObject) * (active_scene.num_models + 1));
		model = &active_scene.models[active_scene.num_models++];
	}else{
		parent->children = realloc(parent->children, sizeof(ModelObject) * (parent->num_children + 1));
		model = &parent->children[parent->num_children++];
	}
	model->parent = parent;
	
	// Name
	model->name = malloc(sizeof(char) * strlen(name));
	strcpy(model->name, name);

	model->num_children = 0;
	model->children = malloc(sizeof(ModelObject) * (model->num_children + 1));
	// Transformation
	if(transform == NULL){
		InitTransform(&model->transform);
	}else{
		model->transform = *transform;
	}

	model->mesh_file = mesh_file;
	model->mesh_index = mesh_index;
	model->shader = shader;
	// memcpy(model->textures, textures, sizeof(int) * 16);


	return model;
}

void InitScene(SceneObject *scene){
	// Allocate space for each thing in the scene

	// Name


	// Mesh files


	// Seperate meshes


	// Textures


	// Shaders


	// Models
	scene->num_models = 0;
	scene->models = malloc(sizeof(ModelObject));

}

void RenderScene(){

}