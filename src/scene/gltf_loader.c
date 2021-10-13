#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SDL.h>

#include "cgltf.h"

cgltf_data *data = NULL;

cgltf_buffer_view *pos;
cgltf_buffer_view *norm;
cgltf_buffer_view *texc;
cgltf_buffer_view *ind;

void LoadGLTF(){
	const char file[] = "../models/axis.gltf";

	cgltf_options options;
	memset(&options, 0, sizeof(cgltf_options));

	cgltf_result result = cgltf_parse_file(&options, file, &data);
	if(result != cgltf_result_success){
		printf("Could not parse file!\n");
	}

	result = cgltf_load_buffers(&options, data, file);
	if(result != cgltf_result_success){
		printf("Could not load buffers!\n");
	}

	result = cgltf_validate(data);
	if(result != cgltf_result_success){
		printf("Invalid file!\n");
	}
	
	// printf("Result: %d\n", result);
	// if (result == cgltf_result_success)
	// {
	// 	printf("Type: %u\n", data->file_type);
	// 	printf("Meshes: %u\n", (unsigned)data->meshes_count);
	// }


	pos = data->meshes->primitives->attributes[0].data->buffer_view;
	norm = data->meshes->primitives->attributes[1].data->buffer_view;
	texc = data->meshes->primitives->attributes[2].data->buffer_view;
	// printf("test: %d\n", data->meshes->primitives->attributes[1].data->buffer_view->offset);
	ind = data->meshes->primitives->indices->buffer_view;

	// printf("%d\n", data->meshes->primitives->attributes[1].data->count);

	// cgltf_buffer_view *buffer = norm;

	// for(int i = buffer->offset; i < (buffer->offset + buffer->size); i += 4){
	// 	float *tmp;
	// 	tmp = malloc(4);
	// 	memcpy(tmp, buffer->buffer->data + i, 4);
	// 	printf("%f, \n", tmp[0]);
	// }
}