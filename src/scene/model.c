#include "../global.h"

#include "model.h"

void InitTransform(TransformObject *transform){
	transform->scale = (Vector3){1, 1, 1};
	glm_mat4_identity(transform->result);
}

void CalculateTransform(TransformObject *transform){
	glm_mat4_identity(transform->result);

}