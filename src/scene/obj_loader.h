#ifndef OBJ_LOADER_H_
#define OBJ_LOADER_H_

#include "../global.h"
// typedef struct Vector3{
// 	union{
// 		struct{float x, y, z;};
// 		float v[3];
// 	};
// }Vector3;

typedef struct MeshObject{
	float *data, *v, *vt, *vn;
	int *f;
	unsigned int data_size, v_size, vt_size, vn_size, f_size;
	unsigned int num_vertices, num_faces, num_meshes;
	bool has_texture_indices;
	bool has_normal_indices;

}MeshObject;

typedef struct MaterialObject{
	char *name; // Material name
	Vector3 ka; // Ambient colour
	Vector3 kd; // Diffuse colour
	Vector3 ks; // Specular colour
	float ns;	// Specular weight
	float d; 	// Transparency
	float ni; 	// Index of refraction
}MaterialObject;

void LoadObj(const char *path, MeshObject *mesh);
void FreeObj(MeshObject *mesh);

#endif