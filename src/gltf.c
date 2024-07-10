#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "debug.h"
#include "vectorlib.h"
#include "json_read.h"

#include "gltf.h"

// #define GLTF_DEBUG

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
						#ifdef GLTF_DEBUG
							printf("Set mesh attribute for attr '%d' to to value '%d'\n", type, attr_token._int);
						#endif
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
			GLTFMesh *mesh_ptr = &gltf_ptr->meshes[gltf_ptr->num_meshes];

			if(json->tokens[token].type == JSMN_OBJECT){
				JSONSetTokenFunc(json, NULL, tfunc_meshes_primitives);
				JSONParse(json);				

			}else{
				switch(JSONTokenHash(json, token, dict_primitives)){
					case 0:; // attributes
						JSONSetTokenFunc(json, NULL, tfunc_meshes_primitives_attributes);
						JSONParse(json);
						break;
					case 1:; // indices
						JSONToken token_value = JSONTokenValue(json, token + 1);
						if(token_value.type == JSON_INT){
							mesh_ptr->has_indices = true;
							mesh_ptr->indices = token_value._int;
						}
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
					#ifdef GLTF_DEBUG
						printf("model: name: '%s'\n", mesh_ptr->name);
					#endif
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
		NULL
	};

	static char *dict_accessors_type[] = {
		"SCALAR",
		"VEC2",
		"VEC3",
		"VEC4",
		"MAT2",
		"MAT3",
		"MAT4",
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
					#ifdef GLTF_DEBUG
						printf("Accessor leading to bufferview: '%d'\n", accessor_ptr->buffer_view);
					#endif
					break;
				case 1: // componentType
					if(t_value.type == JSON_INT){
						accessor_ptr->component_type = t_value._int;
					}
					#ifdef GLTF_DEBUG
						printf("Accessor componenttype: '%d'\n", accessor_ptr->component_type);
					#endif
					break;
				case 2: // count
					if(t_value.type == JSON_INT){
						accessor_ptr->count = t_value._int;
					}
					#ifdef GLTF_DEBUG
						printf("Accessor count: '%d'\n", accessor_ptr->count);
					#endif
					break;
				case 3: // max	< TODO: To be used eventually (Bounding box)
					break;
				case 4: // min	< TODO: To be used eventually (Bounding box)
					break;
				case 5: // type
					switch(JSONTokenHash(json, token + 1, dict_accessors_type)){
						case 0: // SCALAR
							accessor_ptr->type = COMPONENT_SCALAR;
							break;
						case 1: // VEC2
							accessor_ptr->type = COMPONENT_VEC2;
							break;
						case 2: // VEC3
							accessor_ptr->type = COMPONENT_VEC3;
							break;
						case 3: // VEC3
							accessor_ptr->type = COMPONENT_VEC4;
							break;
						case 4: // MAT2
							accessor_ptr->type = COMPONENT_MAT2;
							break;
						case 5: // MAT3
							accessor_ptr->type = COMPONENT_MAT3;
							break;
						case 6: // MAT3
							accessor_ptr->type = COMPONENT_MAT3;
							break;
						
					}
					#ifdef GLTF_DEBUG
						printf("Accessor type: '%d'\n", accessor_ptr->type);
					#endif
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
						#ifdef GLTF_DEBUG
							printf("BufferView buffer: '%d'\n", bufferview_ptr->buffer);
						#endif
						break;
					case 1: // byteLength
						bufferview_ptr->byte_length = t_value._int;
						#ifdef GLTF_DEBUG
							printf("BufferView bytelength: '%d'\n", bufferview_ptr->byte_length);
						#endif
						break;
					case 2: // byteOffset
						bufferview_ptr->byte_offset = t_value._int;
						#ifdef GLTF_DEBUG
							printf("BufferView byteoffset: '%d'\n", bufferview_ptr->byte_offset);
						#endif
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
					#ifdef GLTF_DEBUG
						printf("Buffer bytelength: '%d'\n", buffer_ptr->byte_length);
					#endif
					break;
				case 1: // uri 
					// (only for .gltf not .glb)
					if(t_value.type == JSON_STRING){
						buffer_ptr->uri = NULL;
						JSONTokenToString(json, token + 1, &buffer_ptr->uri);
						#ifdef GLTF_DEBUG
							printf("Buffer uri: '%s'\n", buffer_ptr->uri);
						#endif
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

static GLTFState GLTFStateOpen(char *path){
	GLTFState gltf;
	gltf.path = NULL;
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
		gltf.path = calloc(1, strlen(path) + 1);
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

				FILE *fp = fopen(buffer_path, "rb");
				if(fp != NULL){
					fseek(fp, 0, SEEK_END);
					long file_length = ftell(fp);
					fseek(fp, 0, SEEK_SET);
					if(file_length > -1){
						tmp_buffer_ptr->data = malloc(file_length + 1);
						if(tmp_buffer_ptr->data != NULL){
							size_t read_error_check = fread(tmp_buffer_ptr->data, 1, file_length, fp);
							if(read_error_check != file_length){
								DebugLog(D_WARN, "%s: error reading data from .bin file", path);
								free(tmp_buffer_ptr->data);
								tmp_buffer_ptr->data = NULL;
							}
						}
					}else{
						DebugLog(D_WARN, "%s: error reading length of gltf .bin file", path);
					}
					fclose(fp);
					fp = NULL;
				}else{
					DebugLog(D_WARN, "%s: error opening gltf .bin file", path);
				}
				free(buffer_path);
				buffer_path = NULL;
			}

			// Once the json is loaded, we can use the data to calculate other attributes (tangents, normals, etc)


			gltf.is_loaded = true;
		}else{
			DebugLog(D_WARN, "%s: error reading gltf file", path);
			free(gltf.path);
			gltf.path = NULL;
		}

		gltf_ptr = NULL;
	}
	return gltf;
}

static GLTFState GLTFStateNew(){
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

static void GLTFStateFree(GLTFState *gltf){
	if(gltf != NULL && gltf->is_loaded){
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


static unsigned short ComponentTypeToSize(unsigned short component_type){
	unsigned short size = 0; // Size in bytes
	switch(component_type){
		case 5120: size = 1; break; // signed 	byte
		case 5121: size = 1; break; // unsigned byte
		case 5122: size = 2; break; // signed 	short
		case 5123: size = 2; break; // unsigned short
		case 5124: size = 4; break; // signed 	int
		case 5125: size = 4; break; // unsigned int
		case 5126: size = 4; break; // float
		case 5130: size = 8; break; // double
	}
	return size;
}

GLTF GLTFNew(){
	GLTF gltf;
	gltf.gltf_state = GLTFStateNew();
	gltf.path = NULL;
	gltf.num_meshes = 0;
	gltf.meshes = NULL;
	return gltf;
}

static GLMesh GLMeshNew(){
	GLMesh mesh;
	
	mesh.is_loaded = false;
	mesh.vao = 0;
	mesh.ebo = 0;
	mesh.norm_vbo = 0;
	mesh.pos_vbo = 0;
	mesh.tan_vbo = 0;
	mesh.uv0_vbo = 0;
	mesh.uv1_vbo = 0;

	return mesh;
}

Mesh MeshNew(){
	Mesh mesh;

	mesh.data = NULL;
	mesh.data_length = 0;

	mesh.name = NULL;
	mesh.path = NULL;
	mesh.gltf_state = NULL;
	mesh.mesh_index = 0;

	mesh.gl_data = GLMeshNew();
	// size = (size of a single component) * (number of components in an element)

	mesh.index_exists = false;
	mesh.index_size = 1; 
	mesh.index_type = 4;
	mesh.index_gl_type = 5126; // GL_FLOAT
	mesh.index_count = 0;
	mesh.index_bytelength = 0;
	mesh.index_offset = 0;

	mesh.position_exists = false;
	mesh.position_size = 1; 
	mesh.position_type = 4;
	mesh.position_gl_type = 5126; // GL_FLOAT
	mesh.position_count = 0;
	mesh.position_bytelength = 0;
	mesh.position_offset = 0;

	mesh.uv0_exists = false;
	mesh.uv0_size = 1; 
	mesh.uv0_type = 4;
	mesh.uv0_gl_type = 5126; // GL_FLOAT
	mesh.uv0_count = 0;
	mesh.uv0_bytelength = 0;
	mesh.uv0_offset = 0;

	mesh.uv1_exists = false;
	mesh.uv1_size = 1; 
	mesh.uv1_type = 4;
	mesh.uv1_gl_type = 5126; // GL_FLOAT
	mesh.uv1_count = 0;
	mesh.uv1_bytelength = 0;
	mesh.uv1_offset = 0;

	mesh.normal_exists = false;
	mesh.normal_size = 1; 
	mesh.normal_type = 4;
	mesh.normal_gl_type = 5126; // GL_FLOAT
	mesh.normal_count = 0;
	mesh.normal_bytelength = 0;
	mesh.normal_offset = 0;

	mesh.tangent_exists = false;
	mesh.tangent_size = 1; 
	mesh.tangent_type = 4;
	mesh.tangent_gl_type = 5126; // GL_FLOAT
	mesh.tangent_count = 0;
	mesh.tangent_bytelength = 0;
	mesh.tangent_offset = 0;


	return mesh;
}

Mesh MeshFromGLTF(GLTFState *gltf, unsigned int mesh_index){
	Mesh mesh = MeshNew();
	if(gltf != NULL && mesh_index < gltf->num_meshes){
		// TODO: Add support for multiple buffers (*.bin files)
		mesh.data = gltf->buffers[0].data;
		mesh.data_length = gltf->buffers[0].byte_length;

		mesh.name = gltf->meshes[mesh_index].name;
		mesh.path = gltf->path;
		mesh.gltf_state = gltf;
		mesh.mesh_index = mesh_index;

		// Copy index data only if the data exists..
		if((mesh.index_exists = gltf->meshes[mesh_index].has_indices)){
			unsigned int accessor_index = gltf->meshes[mesh_index].indices;
			unsigned int bufferview_index = gltf->accessors[accessor_index].buffer_view;

			unsigned int gl_type = gltf->accessors[accessor_index].component_type;
			unsigned char type = ComponentTypeToSize(gl_type);
			unsigned char size = gltf->accessors[accessor_index].type;
			unsigned int count = gltf->accessors[accessor_index].count;
			unsigned int byte_length = gltf->buffer_views[bufferview_index].byte_length;
			unsigned int offset = gltf->buffer_views[bufferview_index].byte_offset;

			mesh.index_exists = true;
			mesh.index_size = size; 
			mesh.index_type = type;
			mesh.index_gl_type = gl_type;
			mesh.index_count = count;
			mesh.index_bytelength = byte_length;
			mesh.index_offset = offset;
		}

		for(int i = 0; i < gltf->meshes[mesh_index].num_attributes; i++){
			unsigned int accessor_index = gltf->meshes[mesh_index].attributes[i].accessor;
			unsigned int bufferview_index = gltf->accessors[accessor_index].buffer_view;

			unsigned int gl_type = gltf->accessors[accessor_index].component_type;
			unsigned char type = ComponentTypeToSize(gl_type);
			unsigned char size = gltf->accessors[accessor_index].type;
			unsigned int count = gltf->accessors[accessor_index].count;
			unsigned int byte_length = gltf->buffer_views[bufferview_index].byte_length;
			unsigned int offset = gltf->buffer_views[bufferview_index].byte_offset;

			// To calculate byte_length of a section you simply have to do (num_elements * size);

			switch(gltf->meshes[mesh_index].attributes[i].type){
				case GLTF_POSITION:
					mesh.position_exists = true;
					mesh.position_size = size; 
					mesh.position_type = type;
					mesh.position_gl_type = gl_type;
					mesh.position_count = count;
					mesh.position_bytelength = byte_length;
					mesh.position_offset = offset;
					break;
				case GLTF_NORMAL:
					mesh.normal_exists = true;
					mesh.normal_size = size; 
					mesh.normal_type = type;
					mesh.normal_gl_type = gl_type;
					mesh.normal_count = count;
					mesh.normal_bytelength = byte_length;
					mesh.normal_offset = offset;
					break;
				case GLTF_TEXCOORD_0:
					mesh.uv0_exists = true;
					mesh.uv0_size = size; 
					mesh.uv0_type = type;
					mesh.uv0_gl_type = gl_type;
					mesh.uv0_count = count;
					mesh.uv0_bytelength = byte_length;
					mesh.uv0_offset = offset;
					break;
				case GLTF_TEXCOORD_1:
					mesh.uv1_exists = true;
					mesh.uv1_size = size; 
					mesh.uv1_type = type;
					mesh.uv1_gl_type = gl_type;
					mesh.uv1_count = count;
					mesh.uv1_bytelength = byte_length;
					mesh.uv1_offset = offset;
					break;
				case GLTF_TANGENT:
					mesh.tangent_exists = true;
					mesh.tangent_size = size; 
					mesh.tangent_type = type;
					mesh.tangent_gl_type = gl_type;
					mesh.tangent_count = count;
					mesh.tangent_bytelength = byte_length;
					mesh.tangent_offset = offset;
					break;
				default:
					break;
			}
		}
	}
	return mesh;
}


GLTF GLTFOpen(char *path){
	GLTF gltf = GLTFNew();

	if(path != NULL){
		GLTFState state = GLTFStateOpen(path);
		if(state.is_loaded){
			gltf.gltf_state = state;
			gltf.path = state.path;
			gltf.num_meshes = state.num_meshes;

			gltf.meshes = malloc(sizeof(Mesh) * (gltf.num_meshes + 1));
			if(gltf.meshes != NULL){
				for(int i = 0; i < gltf.num_meshes; i++){
					gltf.meshes[i] = MeshFromGLTF(&state, i);
				}
			}
		}
	}

	return gltf;
}

void GLTFReload(GLTF *gltf){
	if(gltf != NULL && gltf->gltf_state.is_loaded){
		char *path = malloc(strlen(gltf->path) + 1);
		memcpy(path, gltf->path, strlen(gltf->path));
		path[strlen(gltf->path)] = 0;

		GLTFFree(gltf);
		*gltf = GLTFOpen(path);
		free(path);
	}
}

void GLTFFree(GLTF *gltf){
	if(gltf != NULL){
		GLTFStateFree(&gltf->gltf_state);
		
		// Free the array of pointers (the meshes were free'd when we called GLTFStateFree())
		free(gltf->meshes);
		gltf->meshes = NULL;
		gltf->num_meshes = 0;


		gltf->path = NULL; // 'gltf.path' is a pointer to 'gltf.gltf_state.path' so 'GLTFStateFree()' frees it, we only need to stop pointing at it
	}
}