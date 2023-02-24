#ifndef GLTF_H_
#define GLTF_H_

#include "vectorlib.h"

typedef enum GLTF_NUM_COMPONENTS{
	COMPONENT_SCALAR	= 1,
	COMPONENT_VEC2		= 2,
	COMPONENT_VEC3		= 3,
	COMPONENT_VEC4		= 4,
	COMPONENT_MAT2		= 4,
	COMPONENT_MAT3		= 9,
	COMPONENT_MAT4		= 16,
}GLTF_NUM_COMPONENTS;


typedef enum GLTF_ATTR_TYPE{GLTF_NONE, GLTF_POSITION, GLTF_NORMAL, GLTF_TEXCOORD_0, GLTF_TEXCOORD_1, GLTF_TANGENT} GLTF_ATTR_TYPE;
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
		GLTF_NUM_COMPONENTS type;
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

	unsigned int num_buffers;
	GLTFBuffer *buffers; // TODO: Eventually add ability to read multiple buffers
}GLTFState;


typedef struct GLMesh{
	bool is_loaded;
	
	unsigned int vao;
	unsigned int ebo;

	unsigned int pos_vbo;
	unsigned int norm_vbo;
	unsigned int uv0_vbo;
	unsigned int uv1_vbo;
	unsigned int tan_vbo;

}GLMesh;

typedef struct Mesh{
	void *data;
	unsigned int data_length;

	char *name;
	char *path;
	GLTFState *gltf_state;
	unsigned int mesh_index;

	GLMesh gl_data; // Data to be filled in when the mesh data is passed over to OpenGL

	/**
	 * Indices
	 * Position
	 * Texture Coord (UV) 0
	 * Texture Coord (UV) 1
	 * Normal
	 * Tangent
	 */

	bool 			index_exists; 		// Set to true if this section exists within the gltf file
	unsigned char 	index_size; 		// Number of primitives in an element (SCALAR, VEC2, VEC3, MAT3, MAT4, ...)
	unsigned int 	index_type; 		// Size in bytes of the type (short = 2, int = 4, double = 8)
	unsigned int 	index_gl_type; 		// eg. GL_FLOAT, GL_INT, ...
	unsigned int 	index_count; 		// Number of elements
	unsigned int	index_bytelength; 	// Length in bytes of the section (_size * _type * _count)
	unsigned int 	index_offset;		// Offset in bytes from the start of 'data'

	bool 			position_exists; 		// Set to true if this section exists within the gltf file
	unsigned char 	position_size; 			// Number of primitives in an element (SCALAR, VEC2, VEC3, MAT3, MAT4, ...)
	unsigned int 	position_type; 			// Size in bytes of the type (short = 2, int = 4, double = 8)
	unsigned int 	position_gl_type; 		// eg. GL_FLOAT, GL_INT, ...
	unsigned int 	position_count; 		// Number of elements
	unsigned int	position_bytelength; 	// Length in bytes of the section (_size * _type * _count)
	unsigned int 	position_offset;		// Offset in bytes from the start of 'data'

	bool 			uv0_exists; 		// Set to true if this section exists within the gltf file
	unsigned char 	uv0_size; 			// Number of primitives in an element (SCALAR, VEC2, VEC3, MAT3, MAT4, ...)	
	unsigned int 	uv0_type; 			// Size in bytes of the type (short = 2, int = 4, double = 8)
	unsigned int 	uv0_gl_type; 		// eg. GL_FLOAT, GL_INT, ...
	unsigned int 	uv0_count; 			// Number of elements
	unsigned int	uv0_bytelength; 	// Length in bytes of the section (_size * _type * _count)
	unsigned int 	uv0_offset;			// Offset in bytes from the start of 'data'
	
	bool 			uv1_exists; 		// Set to true if this section exists within the gltf file
	unsigned char 	uv1_size; 			// Number of primitives in an element (SCALAR, VEC2, VEC3, MAT3, MAT4, ...)	
	unsigned int 	uv1_type; 			// Size in bytes of the type (short = 2, int = 4, double = 8)
	unsigned int 	uv1_gl_type; 		// eg. GL_FLOAT, GL_INT, ...
	unsigned int 	uv1_count; 			// Number of elements
	unsigned int	uv1_bytelength; 	// Length in bytes of the section (_size * _type * _count)
	unsigned int 	uv1_offset;			// Offset in bytes from the start of 'data'

	bool 			normal_exists; 		// Set to true if this section exists within the gltf file
	unsigned char 	normal_size; 		// Number of primitives in an element (SCALAR, VEC2, VEC3, MAT3, MAT4, ...)
	unsigned int 	normal_type; 		// Size in bytes of the type (short = 2, int = 4, double = 8)
	unsigned int 	normal_gl_type; 	// eg. GL_FLOAT, GL_INT, ...
	unsigned int 	normal_count; 		// Number of elements
	unsigned int	normal_bytelength;	// Length in bytes of the section (_size * _type * _count)
	unsigned int 	normal_offset;		// Offset in bytes from the start of 'data'

	bool 			tangent_exists; 	// Set to true if this section exists within the gltf file
	unsigned char 	tangent_size; 		// Number of primitives in an element (SCALAR, VEC2, VEC3, MAT3, MAT4, ...)
	unsigned int 	tangent_type; 		// Size in bytes of the type (short = 2, int = 4, double = 8)
	unsigned int 	tangent_gl_type; 	// eg. GL_FLOAT, GL_INT, ...
	unsigned int 	tangent_count; 		// Number of elements
	unsigned int	tangent_bytelength; // Length in bytes of the section (_size * _type * _count)
	unsigned int 	tangent_offset;		// Offset in bytes from the start of 'data'


}Mesh;

typedef struct GLTF{
	char *path;
	GLTFState gltf_state;

	unsigned int num_meshes;
	Mesh *meshes;
}GLTF;


/**
 *  @return An initialized empty GLTFState
 */
GLTF GLTFNew();

/**
 *  @brief Open, parse, and seperate .gltf file into meshes
 *  @param path - Path to .gltf file to be read
 */
GLTF GLTFOpen(char *path);

/**
 *  @brief Reload the gltf file specified in 'gltf->path' back into '*gltf'
 */
void GLTFReload(GLTF *gltf);

/**
 *  @brief Free gltf data (Meshes array and buffers)
 */
void GLTFFree(GLTF *gltf);

#endif