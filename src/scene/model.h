#ifndef MODEL_H_
#define MODEL_H_

typedef struct TransformObject{
	Vector3 position;
	Vector3 scale;
	Vector3 rotation;
	mat4 result;
}TransformObject;

#endif