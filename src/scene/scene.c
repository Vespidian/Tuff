#include "../global.h"

#include "scene.h"

SceneObject active_scene;

void InitTransform(TransformObject *transform){
	transform->position = (Vector3){0, 0, 0};
	transform->rotation_e = (Vector3){0, 0, 0};
	transform->rotation_q = (Vector4){0, 0, 0};
	transform->scale = (Vector3){1, 1, 1};
	glm_mat4_identity(transform->result);
}

void CalculateTransform(TransformObject *transform){
	// translate, rotate, scale
	mat4 translation;
	mat4 rotation;
	mat4 scale;

	glm_mat4_identity(translation);
	// glm_mat4_identity(rotation);
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
	CalculateTransform(&model->transform);
	glm_mul(transform, model->transform.result, model->transform.result);

	// Loop through children and add this model's transform to them
	for(int i = 0; i < model->num_children; i++){
		UpdateTransformChildren(&model->children[i], model->transform.result);
	}
}

// Should be called once for every root object at scene initialization as well as whenever an object transform is updated
void CalculateModelTransform(ModelObject *model){
	// Calculate this model's raw transformation
	CalculateTransform(&model->transform);

	// Take the parents transformation matrix and multiply it by this child object's matrix (The parent's matrix includes all grandparent transformations)
	if(model->parent != NULL){
		glm_mul(model->parent->transform.result, model->transform.result, model->transform.result);
	}

	// Recursively applies this model's transform to every child
	mat4 empty_mat;
	glm_mat4_identity(empty_mat);
	UpdateTransformChildren(model, empty_mat);
}

ModelObject *NewModel(char *name, ModelObject *parent, TransformObject *transform, unsigned int mesh_file, unsigned int mesh_index, unsigned int material){
	// Model
	ModelObject *model;

	// If parent is null, this model has no parents and thus is a root node
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

	// Children array
	model->num_children = 0;
	model->children = malloc(sizeof(ModelObject) * (model->num_children + 1));
	
	// Transformation
	if(parent == NULL){
		if(transform == NULL){
			// No parent, no transform.. Initialize default transform
			InitTransform(&model->transform);
		}else{
			// No parent, yes transform.. set model transform to specified transform
			model->transform = *transform;
		}
	}else{
		if(transform == NULL){
			// Yes parent, no transform.. set model transform to parent transform
			model->transform = parent->transform;
		}else{
			// Yes parent, yes transform.. multiply model by parent transform
			glm_mul(parent->transform.result, model->transform.result, model->transform.result);
		}
	}

	model->mesh_file = mesh_file;
	model->mesh_index = mesh_index;
	model->material = material;

	return model;
}

ModelObject *FindModel(char *name, ModelObject *start){
	ModelObject *model = NULL;
	if(start == NULL){
		for(int i = 0; i < active_scene.num_models; i++){
			if(strcmp(name, active_scene.models[i].name) == 0){
				model = &active_scene.models[i];
			}else{
				model = FindModel(name, &active_scene.models[i]);
			}
		}
	}else{
		for(int i = 0; i < start->num_children; i++){
			if(strcmp(name, start->children[i].name) == 0){
				model = &start->children[i];
			}else{
				model = FindModel(name, &start->children[i]);
			}
		}
	}
	return model;
}

void InitScene(SceneObject *scene){
	// Allocate space for each data type in the scene

	// Name
	scene->name = malloc(sizeof(char));

	// Mesh files
	scene->num_mesh_files = 0;
	scene->mesh_files = malloc(sizeof(cgltf_data));

	// Seperate meshes
	scene->num_meshes = 0;
	scene->meshes = malloc(sizeof(MeshObject));

	// Textures
	scene->num_textures = 0;
	scene->textures = malloc(sizeof(TextureObject));

	// Shaders
	scene->num_shaders = 0;
	scene->shaders = malloc(sizeof(ShaderObject));

	// Materials
	scene->num_materials = 0;
	scene->materials = malloc(sizeof(MaterialObject));

	// Models
	scene->num_models = 0;
	scene->models = malloc(sizeof(ModelObject));

}

// void LoadScene(char *name, char *path, SceneObject *scene){

// }

void RenderScene(){

}