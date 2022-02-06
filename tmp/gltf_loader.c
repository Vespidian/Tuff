#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "cgltf.h"
#include "../vectorlib.h"
#include "../json_base.h"

cgltf_data *data = NULL;

cgltf_buffer_view *pos;
cgltf_buffer_view *norm;
cgltf_buffer_view *texc;
cgltf_buffer_view *ind;

cgltf_buffer_view *tanv;
int tanv_size = 0;

static char *gltf_dict[] = {
	"meshes",
	"accessors",
	"bufferViews",
	"buffers",
	NULL
};

typedef enum GLTF_ATTR_TYPE{GLTF_NONE, GLTF_POSITION, GLTF_NORMAL, GLTF_TEXCOORD_0, GLTF_TEXCOORD_1} GLTF_ATTR_TYPE;
		typedef struct GLTFAttribute{
			GLTF_ATTR_TYPE type;
			unsigned int accessor;
		}GLTFAttribute;

	typedef struct GLTFMesh{
		char *name;
		
		unsigned int num_attributes;
		GLTFAttribute *attributes;

		bool has_indices;
		unsigned int indices;
	}GLTFMesh;

	typedef struct GLTFAccessor{
		unsigned int buffer_view;
		unsigned int component_type;
		unsigned int count;
		Vector3 max;
		Vector3 min;
		unsigned int type;
	}GLTFAccessor;

	typedef struct GLTFBufferView{
		unsigned int buffer;
		unsigned int byte_length;
		unsigned int byte_offset;
	}GLTFBufferView;

typedef struct GLTFState{
	char *path;

	unsigned int num_meshes;
	GLTFMesh *meshes;

	unsigned int num_accessors;
	GLTFAccessor *accessors;

	unsigned int num_bufferviews;
	GLTFBufferView *buffer_views;

	void *buffer; // TODO: Eventually add ability to read multiple buffers
	unsigned int buffer_length;
}GLTFState;

static GLTFState *gltf_ptr;

	static GLTFMesh *mesh_ptr = NULL;
		static char *dict_meshes_attributes[] = {
			"POSITION",
			"NORMAL",
			"TEXCOORD_0",
			"TEXCOORD_1",
			"TANGENT", // possibly wrong
			NULL
		};
		static void tfunc_meshes_attributes(JSONState *json, unsigned int token){
			if(mesh_ptr != NULL){
				JSONToken attr_token = JSONTokenValue(json, token + 1);
				if(attr_token.type == JSON_INT){
					bool exists = false;
					GLTF_ATTR_TYPE type = GLTF_NONE;
					switch(JSONTokenHash(json, token, dict_meshes_attributes)){
						case 0: // POSITION
							type = GLTF_POSITION;
							break;
						case 1: // NORMAL
							type = GLTF_NORMAL;
							break;
						case 2: // TEXCOORD_0
							type = GLTF_TEXCOORD_0;
							break;
						case 3: // TEXCOORD_1
							type = GLTF_TEXCOORD_1;
							break;
						case 4: // TANGENT
							// type = GLTF_TEXCOORD_1;
							break;
					}
					if(exists){
						GLTFAttribute *tmp_attr = realloc(mesh_ptr->attributes, sizeof(GLTFAttribute) * (mesh_ptr->num_attributes + 1));
						if(tmp_attr != NULL){
							mesh_ptr->attributes = tmp_attr;
							mesh_ptr->attributes[mesh_ptr->num_attributes].type = type;
							mesh_ptr->attributes[mesh_ptr->num_attributes].accessor = attr_token._int;
							printf("Set mesh attribute for attr '%d' to to value '%d'", type, attr_token._int);
						}
					}
				}
			}
		}
	static char *dict_meshes[] = {
		"name",
		"primitives",
			"attributes",
			"indices",
			"",
		NULL
	};
	static void tfunc_meshes(JSONState *json, unsigned int token){
		if(mesh_ptr != NULL){
			switch(JSONTokenHash(json, token, dict_meshes)){
				case 0:; // name
					JSONToken t_value = JSONTokenValue(json, token + 1);
					if(t_value.type == JSON_STRING){
						mesh_ptr->name = NULL;
						JSONTokenToString(json, token + 1, &mesh_ptr->name);
					}
					break;
				case 1: // primitives
					JSONSetTokenFunc(json, NULL, tfunc_meshes);
					JSONParse(json);
					break;
				case 2: // attributes
					JSONSetTokenFunc(json, NULL, tfunc_meshes_attributes);
					JSONParse(json);
					break;
				case 3: // indices
					break;
			}
		}
	}

	static char *dict_accessors[] = {
		"bufferView",
		"componentType",
		"count",
		"max",
		"min",
		"type",
		NULL
	};
	static void tfunc_accessors(JSONState *json, unsigned int token){
		switch(JSONTokenHash(json, token, dict_accessors)){
			case 0: // bufferView
				break;
			case 1: // componentType
				break;
			case 2: // count
				break;
			case 3: // max
				break;
			case 4: // min
				break;
			case 5: // type
				break;
		}
	}

	static char *dict_bufferviews[] = {
		"buffer",
		"byteLength",
		"byteOffset",
		"",
		NULL
	};
	static void tfunc_bufferviews(JSONState *json, unsigned int token){
		switch(JSONTokenHash(json, token, dict_bufferviews)){
			case 0: // buffer
				break;
			case 1: // byteLength
				break;
			case 2: // byteOffset
				break;
		}
	}

	static char *dict_buffers[] = {
		"byteLength",
		"uri",
		"",
		NULL
	};
	static void tfunc_buffers(JSONState *json, unsigned int token){
		switch(JSONTokenHash(json, token, dict_buffers)){
			case 0: // byteLength
				break;
			case 1: // uri
				break;
		}
	}

void tfunc_gltf(JSONState *json, unsigned int token){
	if(gltf_ptr != NULL){
		switch(JSONTokenHash(json, token, gltf_dict)){
			case 0:; // meshes
				GLTFMesh *tmp_meshes = realloc(gltf_ptr->meshes, sizeof(GLTFMesh) * (gltf_ptr->num_meshes + 1));
				if(tmp_meshes != NULL){
					gltf_ptr->meshes = tmp_meshes;
					mesh_ptr = &gltf_ptr->meshes[gltf_ptr->num_meshes];
					mesh_ptr->name = NULL;
					mesh_ptr->num_attributes = 0;
					mesh_ptr->attributes = NULL;
					mesh_ptr->has_indices = false;

					JSONSetTokenFunc(json, NULL, tfunc_meshes);
					JSONParse(json);

					mesh_ptr = NULL;

					gltf_ptr->num_meshes++;
				}
				break;
			case 1: // accessors
				JSONSetTokenFunc(json, NULL, tfunc_accessors);
				JSONParse(json);
				break;
			case 2: // bufferViews
				JSONSetTokenFunc(json, NULL, tfunc_bufferviews);
				JSONParse(json);
				break;
			case 3: // buffers
				JSONSetTokenFunc(json, NULL, tfunc_buffers);
				JSONParse(json);
				break;
		}
	}
}

void LoadGLTF(char *path){
	if(path != NULL){
		GLTFState gltf;
		
		gltf.path = malloc(strlen(path) + 1);
		if(gltf.path != NULL){
			memcpy(gltf.path, path, strlen(path));
			gltf.path[strlen(path)] = 0;
		}

		gltf.num_meshes = 0;
		gltf.meshes = NULL;

		gltf.num_accessors = 0;
		gltf.accessors = NULL;

		gltf.num_bufferviews = 0;
		gltf.buffer_views = NULL;

		gltf.buffer_length = 0;
		gltf.buffer = NULL;

		gltf_ptr = &gltf;

		JSONState json = JSONOpen(path);
		JSONSetTokenFunc(&json, NULL, tfunc_gltf);
		JSONParse(&json);
		JSONFree(&json);

		gltf_ptr = NULL;
	}

	// const char file[] = "meshes/entrance.gltf";

	// cgltf_options options;
	// memset(&options, 0, sizeof(cgltf_options));

	// cgltf_result result = cgltf_parse_file(&options, file, &data);
	// if(result != cgltf_result_success){
	// 	printf("Could not parse file!\n");
	// }

	// result = cgltf_load_buffers(&options, data, file);
	// if(result != cgltf_result_success){
	// 	printf("Could not load buffers!\n");
	// }

	// result = cgltf_validate(data);
	// if(result != cgltf_result_success){
	// 	printf("Invalid file!\n");
	// }
	
	// printf("Result: %d\n", result);
	// if (result == cgltf_result_success)
	// {
	// 	printf("Type: %u\n", data->file_type);
	// 	printf("Meshes: %u\n", (unsigned)data->meshes_count);
	// }


	// pos = data->meshes->primitives->attributes[0].data->buffer_view;
	// norm = data->meshes->primitives->attributes[1].data->buffer_view;
	// texc = data->meshes->primitives->attributes[2].data->buffer_view;
	// // printf("test: %d\n", data->meshes->primitives->attributes[1].data->buffer_view->offset);
	// ind = data->meshes->primitives->indices->buffer_view;


	// Calculate tangent:
	// for(int i = 0; i < ind->size; i += 3){ // Loop over triangles
	// 	Vector3 vert[3];
	// 	vert[0] = pos->buffer->data[(data->buffers->data + i + 0)];
	// 	vert[1] = pos->buffer->data[(data->buffers->data + i + 1)];
	// 	vert[2] = pos->buffer->data[(data->buffers->data + i + 2)];
	// }

	// printf("%d\n", data->meshes->primitives->attributes[1].data->count);

	// cgltf_buffer_view *buffer = norm;

	// for(int i = buffer->offset; i < (buffer->offset + buffer->size); i += 4){
	// 	float *tmp;
	// 	tmp = malloc(4);
	// 	memcpy(tmp, buffer->buffer->data + i, 4);
	// 	printf("%f, \n", tmp[0]);
	// }
}