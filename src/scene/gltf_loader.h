#ifndef GLTF_LOADER_H_
#define GLTF_LOADER_H_

#include "cgltf.h"

cgltf_data *data;

cgltf_buffer_view *pos;
cgltf_buffer_view *norm;
cgltf_buffer_view *texc;
cgltf_buffer_view *ind;


void LoadGLTF();

#endif