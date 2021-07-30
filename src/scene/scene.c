#include "../global.h"

#include "scene.h"

SceneObject active_scene;

void InitTransform(TransformObject *transform){
	transform->scale = (Vector3){1, 1, 1};
	glm_mat4_identity(transform->result);
}

void CalculateTransform(TransformObject *transform){
	glm_mat4_identity(transform->result);
	// glm_quat()
}

void InitScene(SceneObject *scene){
	// Allocate space for each thing in the scene

	// Name


	// Mesh files


	// Seperate meshes


	// Textures


	// Shaders


	// Models


}

void RenderScene(){

}