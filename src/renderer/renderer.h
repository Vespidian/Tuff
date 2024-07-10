#ifndef RENDERER_H_
#define RENDERER_H_

#include "../gltf.h"

// VAO function constants
enum VAOAttributes{ATTR_FLOAT = 1, ATTR_VEC2, ATTR_VEC3, ATTR_VEC4, ATTR_MAT2 = 8, ATTR_MAT3 = 9, ATTR_MAT4 = 16};

/**
 *  Stores Vertex Attribute Object data of a VAO created with 'NewVAO()'
 */
typedef struct AttribArray{
	unsigned int array_object;
	unsigned int stride;
	unsigned int num_attrib_slots;
	unsigned int instanced_vbo;
}AttribArray;

/**
 *  Stores all the data needed to render an instance
 */
typedef struct RendererInstance{
	char num_textures_used;
	unsigned int texture[16];
	Mesh mesh;
	Shader *shader;
	AttribArray vao;
	unsigned int count;
	float *buffer;
}RendererInstance;

/**
 *  Quad texture coordinates
 */
extern mat4 default_texture_coordinates;

/**
 *  Number of calls to 'AppendInstance()' in current frame
 */
extern int num_append_instance_calls;

/**
 *  @brief Dynamically create a VAO
 *  @param num_attribs number of ATTR_... enums specified
 *  @param ... attribute enums
 *  @return AttribArray with VAO data
 */
// AttribArray NewVAO(int num_attribs, ...);
AttribArray NewVAO(unsigned int input_vao, unsigned int start_attrib_index, int num_attribs, ...);

/**
 *  @brief Send instance data to the instance_buffer
 *  @param vao the vao you want to render with
 *  @param data[64] vertex data to put in instance vbo
 *  @param shader reference to desired shader
 *  @param num_textures_used number of textures to copy to instance texture buffer
 *  @param textures[16] array of textures to use
 */
void AppendInstance(AttribArray vao, float data[64], Mesh mesh, Shader *shader, char num_textures_used, Texture textures[16]);

/**
 *  @brief Passes all instance data to the GPU (window buffer still needs to be swapped)
 */
void PushRender();


#endif
