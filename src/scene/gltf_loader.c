#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// #include <SDL2/SDL.h>

// #include "cgltf.h"
#include "../debug.h"
#include "../vectorlib.h"
#include "../json_base.h"

#include "gltf_loader.h"

static GLTFState *gltf_ptr = NULL;
		static char *dict_meshes_attributes[] = {
			"POSITION",
			"NORMAL",
			"TEXCOORD_0",
			"TEXCOORD_1",
			"TANGENT", // possibly wrong
			NULL
		};
		
		static void tfunc_meshes_primitives_attributes(JSONState *json, unsigned int token){
			if(gltf_ptr != NULL){
				GLTFMesh *mesh_ptr = &gltf_ptr->meshes[gltf_ptr->num_meshes];
				JSONToken attr_token = JSONTokenValue(json, token + 1);
				bool exists = false;
				GLTF_ATTR_TYPE type = GLTF_NONE;
				switch(JSONTokenHash(json, token, dict_meshes_attributes)){
					case 0: // POSITION
						type = GLTF_POSITION;
						exists = true;
						break;
					case 1: // NORMAL
						type = GLTF_NORMAL;
						exists = true;
						break;
					case 2: // TEXCOORD_0
						type = GLTF_TEXCOORD_0;
						exists = true;
						break;
					case 3: // TEXCOORD_1
						type = GLTF_TEXCOORD_1;
						exists = true;
						break;
					case 4: // TANGENT
						// type = GLTF_TEXCOORD_1;
						exists = true;
						break;
				}
				if(exists){
					GLTFAttribute *tmp_attr = realloc(mesh_ptr->attributes, sizeof(GLTFAttribute) * (mesh_ptr->num_attributes + 1));
					if(tmp_attr != NULL){
						mesh_ptr->attributes = tmp_attr;
						mesh_ptr->attributes[mesh_ptr->num_attributes].type = type;
						mesh_ptr->attributes[mesh_ptr->num_attributes].accessor = attr_token._int;
						printf("Set mesh attribute for attr '%d' to to value '%d'\n", type, attr_token._int);
						mesh_ptr->num_attributes++;
					}
				}
			}
		}

		static char *dict_primitives[] = {
			"attributes",
			"indices",
			NULL
		};
		static void tfunc_meshes_primitives(JSONState *json, unsigned int token){
			// GLTFMesh *mesh_ptr = &gltf_ptr->meshes[gltf_ptr->num_meshes];

			if(json->tokens[token].type == JSMN_OBJECT){
				JSONSetTokenFunc(json, NULL, tfunc_meshes_primitives);
				JSONParse(json);				

			}else{
				switch(JSONTokenHash(json, token, dict_primitives)){
					case 0:; // attributes
						JSONSetTokenFunc(json, NULL, tfunc_meshes_primitives_attributes);
						JSONParse(json);
						break;
					case 1: // indices
						break;
				}
			}
		}
	static char *dict_meshes[] = {
		"name",
		"primitives",
		NULL
	};
	static void tfunc_meshes(JSONState *json, unsigned int token){
		GLTFMesh *mesh_ptr = &gltf_ptr->meshes[gltf_ptr->num_meshes];
		if(json->tokens[token].type == JSMN_OBJECT){

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

				gltf_ptr->num_meshes++;
			}
		}else{
			switch(JSONTokenHash(json, token, dict_meshes)){
				case 0:; // name
					mesh_ptr->name = NULL;
					JSONTokenToString(json, token + 1, &mesh_ptr->name);
					printf("model: name: '%s'\n", mesh_ptr->name);
					break;
				case 1:; // primitives
					JSONSetTokenFunc(json, NULL, tfunc_meshes_primitives);
					JSONParse(json);
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

		"SCALAR",
		"VEC2",
		"VEC3",
		NULL
	};
	static void tfunc_accessors(JSONState *json, unsigned int token){
		GLTFAccessor *accessor_ptr = &gltf_ptr->accessors[gltf_ptr->num_accessors];
		if(json->tokens[token].type == JSMN_OBJECT){

			GLTFAccessor *tmp_accessor = realloc(gltf_ptr->accessors, sizeof(GLTFAccessor) * (gltf_ptr->num_accessors + 1));
			if(tmp_accessor != NULL){
				gltf_ptr->accessors = tmp_accessor;

				accessor_ptr = &gltf_ptr->accessors[gltf_ptr->num_accessors];
				accessor_ptr->buffer_view = 0;
				accessor_ptr->component_type = 5126; // GL_FLOAT
				accessor_ptr->count = 0;
				accessor_ptr->max = (Vector3){0, 0, 0};
				accessor_ptr->min = (Vector3){0, 0, 0};
				accessor_ptr->type = 0; // TODO: define an enum with various types (vec3, mat4, ..)

				JSONSetTokenFunc(json, NULL, tfunc_accessors);
				JSONParse(json);

				gltf_ptr->num_accessors++;
			}
		}else{
			JSONToken t_value = JSONTokenValue(json, token + 1);
			switch(JSONTokenHash(json, token, dict_accessors)){
				case 0: // bufferView
					if(t_value.type == JSON_INT){
						accessor_ptr->buffer_view = t_value._int;
					}
					printf("Accessor leading to bufferview: '%d'\n", accessor_ptr->buffer_view);
					break;
				case 1: // componentType
					if(t_value.type == JSON_INT){
						accessor_ptr->component_type = t_value._int;
					}
					printf("Accessor componenttype: '%d'\n", accessor_ptr->component_type);
					break;
				case 2: // count
					if(t_value.type == JSON_INT){
						accessor_ptr->count = t_value._int;
					}
					printf("Accessor count: '%d'\n", accessor_ptr->count);
					break;
				case 3: // max	< TODO: To be used eventually (Bounding box)
					break;
				case 4: // min	< TODO: To be used eventually (Bounding box)
					break;
				case 5: // type
					switch(JSONTokenHash(json, token + 1, dict_accessors)){
						case 6: // SCALAR
							accessor_ptr->type = SCALAR;
							break;
						case 7: // VEC2
							accessor_ptr->type = VEC2;
							break;
						case 8: // VEC3
							accessor_ptr->type = VEC3;
							break;
					}
					printf("Accessor type: '%d'\n", accessor_ptr->type);
					break;
			}
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
		GLTFBufferView *bufferview_ptr = &gltf_ptr->buffer_views[gltf_ptr->num_bufferviews];
		if(json->tokens[token].type == JSMN_OBJECT){

			GLTFBufferView *tmp_bufferview = realloc(gltf_ptr->buffer_views, sizeof(GLTFBufferView) * (gltf_ptr->num_bufferviews + 1));
			if(tmp_bufferview != NULL){
				gltf_ptr->buffer_views = tmp_bufferview;

				bufferview_ptr = &gltf_ptr->buffer_views[gltf_ptr->num_bufferviews];
				bufferview_ptr->buffer = 0;
				bufferview_ptr->byte_length = 0;
				bufferview_ptr->byte_offset = 0;

				JSONSetTokenFunc(json, NULL, tfunc_bufferviews);
				JSONParse(json);

				gltf_ptr->num_bufferviews++;
			}
		}else{
			JSONToken t_value = JSONTokenValue(json, token + 1);
			if(t_value.type == JSON_INT){
				switch(JSONTokenHash(json, token, dict_bufferviews)){
					case 0: // buffer
						bufferview_ptr->buffer = t_value._int;
						printf("BufferView buffer: '%d'\n", bufferview_ptr->buffer);
						break;
					case 1: // byteLength
						bufferview_ptr->byte_length = t_value._int;
						printf("BufferView bytelength: '%d'\n", bufferview_ptr->byte_length);
						break;
					case 2: // byteOffset
						bufferview_ptr->byte_offset = t_value._int;
						printf("BufferView byteoffset: '%d'\n", bufferview_ptr->byte_offset);
						break;
				}
			}
		}
	}

	static char *dict_buffers[] = {
		"byteLength",
		"uri",
		"",
		NULL
	};
	static void tfunc_buffers(JSONState *json, unsigned int token){
		GLTFBuffer *buffer_ptr = &gltf_ptr->buffers[gltf_ptr->num_buffers];
		if(json->tokens[token].type == JSMN_OBJECT){

			GLTFBuffer *tmp_buffer = realloc(gltf_ptr->buffers, sizeof(GLTFBuffer) * (gltf_ptr->num_buffers + 1));
			if(tmp_buffer != NULL){
				gltf_ptr->buffers = tmp_buffer;

				buffer_ptr = &gltf_ptr->buffers[gltf_ptr->num_buffers];
				buffer_ptr->byte_length = 0;
				buffer_ptr->uri = NULL;
				buffer_ptr->data = NULL;

				JSONSetTokenFunc(json, NULL, tfunc_buffers);
				JSONParse(json);

				gltf_ptr->num_buffers++;
			}
		}else{
			JSONToken t_value = JSONTokenValue(json, token + 1);
			switch(JSONTokenHash(json, token, dict_buffers)){
				case 0: // byteLength
					if(t_value.type == JSON_INT){
						buffer_ptr->byte_length = t_value._int;
					}
					printf("Buffer bytelength: '%d'\n", buffer_ptr->byte_length);
					break;
				case 1: // uri 
					// (only for .gltf not .glb)
					if(t_value.type == JSON_STRING){
						buffer_ptr->uri = malloc(strlen(t_value._string) + 1);
						memcpy(buffer_ptr->uri, t_value._string, strlen(t_value._string));
						buffer_ptr->uri[strlen(t_value._string)] = 0;

						printf("Buffer uri: '%s'\n", buffer_ptr->uri);
					}
					break;
			}
		}
	}

static char *gltf_dict[] = {
	"meshes",
	"accessors",
	"bufferViews",
	"buffers",
	NULL
};
void tfunc_gltf(JSONState *json, unsigned int token){
	if(gltf_ptr != NULL){
		if(json->tokens[token + 1].type == JSMN_ARRAY){
			switch(JSONTokenHash(json, token, gltf_dict)){
				case 0:; // meshes
					gltf_ptr->meshes = malloc(sizeof(GLTFMesh));
					JSONSetTokenFunc(json, NULL, tfunc_meshes);
					JSONParse(json);
					break;
				case 1:; // accessors
					gltf_ptr->accessors = malloc(sizeof(GLTFAccessor));
					JSONSetTokenFunc(json, NULL, tfunc_accessors);
					JSONParse(json);
					break;
				case 2:; // bufferViews
					gltf_ptr->buffer_views = malloc(sizeof(GLTFBufferView));
					JSONSetTokenFunc(json, NULL, tfunc_bufferviews);
					JSONParse(json);
					break;
				case 3:; // buffers
					gltf_ptr->buffers = malloc(sizeof(GLTFBuffer));
					JSONSetTokenFunc(json, NULL, tfunc_buffers);
					JSONParse(json);
					break;
			}
		}
	}
}

GLTFState GLTFOpen(char *path){
	GLTFState gltf;
	gltf.is_loaded = false;

	gltf.num_meshes = 0;
	gltf.meshes = NULL;

	gltf.num_accessors = 0;
	gltf.accessors = NULL;

	gltf.num_bufferviews = 0;
	gltf.buffer_views = NULL;

	gltf.num_buffers = 0;
	gltf.buffers = NULL;

	if(path != NULL){
		gltf.path = malloc(strlen(path) + 1);
		if(gltf.path != NULL){
			memcpy(gltf.path, path, strlen(path));
			gltf.path[strlen(path)] = 0;
		}

		gltf_ptr = &gltf;

		JSONState json = JSONOpen(path);
		if(json.is_loaded){
			JSONSetTokenFunc(&json, NULL, tfunc_gltf);
			JSONParse(&json);
			JSONFree(&json);

			// Load in the seperate files holding binary data
			for(int i = 0; i < gltf.num_buffers; i++){
				GLTFBuffer *tmp_buffer_ptr = &gltf.buffers[i];

				int path_length = strrchr(gltf.path, '/') + 1 - gltf.path;
				char *buffer_path = malloc((path_length) + strlen(tmp_buffer_ptr->uri) + 1);
				memcpy(buffer_path, gltf.path, path_length); // Copy the base path of the gltf first
				memcpy(buffer_path + path_length, tmp_buffer_ptr->uri, strlen(tmp_buffer_ptr->uri)); // Now copy the filename
				buffer_path[(path_length) + strlen(tmp_buffer_ptr->uri)] = 0;

				FILE *fp = fopen(buffer_path, "rb"); // TODO: This currently takes the path relative to the parent .gltf file (we either fix this OR only support .glb files)
				if(fp != NULL){
					fseek(fp, 0, SEEK_END);
					long file_length = ftell(fp);
					fseek(fp, 0, SEEK_SET);
					if(file_length > -1){
						tmp_buffer_ptr->data = malloc(file_length + 1);
						if(tmp_buffer_ptr->data != NULL){
							size_t read_error_check = fread(tmp_buffer_ptr->data, 1, file_length, fp);
							if(read_error_check != file_length){
								DebugLog(D_WARN, "%s: error reading data from file", buffer_path);
								free(tmp_buffer_ptr->data);
								tmp_buffer_ptr->data = NULL;
							}
						}
					}else{
						DebugLog(D_WARN, "%s: error reading length of JSON file", buffer_path);
					}
					fclose(fp);
				}else{
					DebugLog(D_WARN, "%s: error opening gltf .bin file", buffer_path);
				}
			}

			// Once the json is loaded, we can use the data to calculate other attributes (tangents, normals, etc)


			gltf.is_loaded = true;
		}else{
			DebugLog(D_WARN, "%s: error reading gltf file", path);
		}

		gltf_ptr = NULL;
	}
	return gltf;
}

GLTFState GLTFNew(){
	GLTFState gltf;
	gltf.is_loaded = false;

	gltf.num_meshes = 0;
	gltf.meshes = NULL;

	gltf.num_accessors = 0;
	gltf.accessors = NULL;

	gltf.num_bufferviews = 0;
	gltf.buffer_views = NULL;

	gltf.num_buffers = 0;
	gltf.buffers = NULL;

	gltf.path = NULL;

	return gltf;
}

void GLTFFree(GLTFState *gltf){
	if(gltf != NULL){
		if(gltf->is_loaded){
			gltf->is_loaded = false;

			free(gltf->path);
			gltf->path = NULL;

			if(gltf->meshes != NULL){
				for(int i = 0; i < gltf->num_meshes; i++){
					free(gltf->meshes[i].name);
					gltf->meshes[i].name = NULL;
					free(gltf->meshes[i].attributes);
					gltf->meshes[i].attributes = NULL;
				}
				// TODO: Fix crash on exit
				free(gltf->meshes);
				gltf->meshes = NULL;
			}

			free(gltf->accessors);
			gltf->accessors = NULL;

			free(gltf->buffer_views);
			gltf->buffer_views = NULL;

			if(gltf->buffers != NULL){
				for(int i = 0; i < gltf->num_buffers; i++){
					free(gltf->buffers[i].uri);
					gltf->buffers[i].uri = NULL;
					free(gltf->buffers[i].data);
					gltf->buffers[i].data = NULL;
				}
				free(gltf->buffers);
				gltf->buffers = NULL;
			}
		}
	}
}