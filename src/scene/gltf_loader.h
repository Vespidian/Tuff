#ifndef GLTF_LOADER_H_
#define GLTF_LOADER_H_

#include "../vectorlib.h"

enum{SCALAR, FLOAT, VEC2, VEC3, VEC4};

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

	typedef struct GLTFBuffer{
		unsigned int byte_length;
		char *uri;
		void *data;
	}GLTFBuffer;

typedef struct GLTFState{
	char *path;

	bool is_loaded;

	unsigned int num_meshes;
	GLTFMesh *meshes;

	unsigned int num_accessors;
	GLTFAccessor *accessors;

	unsigned int num_bufferviews;
	GLTFBufferView *buffer_views;

	// void *buffer; // TODO: Eventually add ability to read multiple buffers
	// unsigned int buffer_length;
	unsigned int num_buffers;
	GLTFBuffer *buffers;
}GLTFState;


/**
 * 
 */
GLTFState GLTFOpen(char *path);

/**
 *  @return An initialized empty GLTFState
 */
GLTFState GLTFNew();

/**
 * 
 */
void GLTFFree(GLTFState *gltf);

#endif