#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#include "../debug.h"
#include "../textures.h"
#include "../shader.h"
#include "../gltf.h"

#include "renderer.h"


#define GLCall CheckGLErrors(__FILE__, __LINE__);

/**
 *  Active VAO
 */
unsigned int current_vao;

/**
 *  Ordered array of all bound textures
 */
unsigned int bound_textures[16];

/**
 *  Bound texture unit
 */
unsigned int current_texture_unit;

void CheckGLErrors(const char *file, int line){
    unsigned int error_code;
    while((error_code = glGetError()) != GL_NO_ERROR){
        char error_string[32];
        switch(error_code){
            case 1280:
                strcpy(error_string, "INVALID_ENUM");
                break;
            case 1281:
                strcpy(error_string, "INVALID_VALUE");
                break;
            case 1282:
                strcpy(error_string, "INVALID_OPERATION");
                break;
            case 1283:
                strcpy(error_string, "STACK_OVERFLOW");
                break;
            case 1284:
                strcpy(error_string, "STACK_UNDERFLOW");
                break;
            case 1285:
                strcpy(error_string, "OUT_OF_MEMORY");
                break;
            case 1286:
                strcpy(error_string, "INVALID_FRAMEBUFFER_OPERATION");
                break;
			default:
				break;
        }
        DebugLog(D_ERR, "Opengl error '%s' ('%s': %d)", error_string, file, line);
        printf("Opengl error '%s' ('%s': %d)", error_string, file, line);
    }
}


RendererInstance *instance_buffer;
unsigned int num_instances = 0;
int num_append_instance_calls = 0;

AttribArray NewVAO(unsigned int input_vao, unsigned int start_attrib_index, int num_attribs, ...){
	// Initialize vao variables
	AttribArray vao;
	vao.num_attrib_slots = 0;
	vao.stride = 0;

	// Maximum of 16 vertex attributes
	if(num_attribs > 16){
		num_attribs = 16;
	}

	// Bind the vertex array object that was passed in
	vao.array_object = input_vao;
	GLCall(glBindVertexArray(vao.array_object));

	// Setting attribute locations for 'instanced_vbo'
	// This is the vbo which contains the per instance data (positions, colors, etc)
	// We set up each attribute of this below based on the passed in function args
	GLCall(glGenBuffers(1, &vao.instanced_vbo));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, vao.instanced_vbo));

	uint8_t attribs[16];
	va_list vl;
	va_start(vl, num_attribs);
	// Retrieve through attributes and calculate stride
	for(int i = 0; i < num_attribs; i++){
		attribs[i] = va_arg(vl, int);
		vao.stride += attribs[i];
	}
	va_end(vl);

	// Loop through attributes and enable them
	unsigned int attrib_offset = 0;
	unsigned int attrib_index = start_attrib_index;
	for(int i = 0; i < num_attribs; i++){
		// If the attribute is a matrix, 'mat_type' defines the number of vertex attribs it consumes
		int mat_type = 0;
		switch(attribs[i]){
			case ATTR_MAT4:
				mat_type = 4;
				break;
			case ATTR_MAT3:
				mat_type = 3;
				break;
			case ATTR_MAT2:
				mat_type = 2;
				attribs[i] = 4;
				break;
			default:
				mat_type = 1;
				break;
		}
		// For matrices, loops through each vertex attribute, otherwise only executes once
		for(int j = 0; j < mat_type; j++){
			GLCall(glEnableVertexAttribArray(attrib_index));

			// Size is 'mat_type' for matrixes and 'attribs[i]' for normal attributes
			GLCall(glVertexAttribPointer(
				attrib_index, 
				(mat_type != 1) ? mat_type : attribs[i], 
				GL_FLOAT, 
				GL_FALSE, 
				vao.stride * sizeof(float), 
				(void*)(attrib_offset * sizeof(float))
			));
			GLCall(glVertexAttribDivisor(attrib_index, 1));
			
			attrib_index++;
			
			// mat2 needs to be differentiated from vec4 (both have 4 variables)
			if(mat_type != 1){
				attrib_offset += mat_type;
			}else{
				attrib_offset += attribs[i];
			}
		}
		
		// 'mat_type' is always equal to the number of attributes consumed
		vao.num_attrib_slots += mat_type;

		// Set mat2 value back to normal for future use
		if(attribs[i] == 4 && mat_type == 2){
			attribs[i] = 8;
		}
	}
	
	return vao;
}

unsigned int NewInstance(char num_textures_used, Texture textures[16], AttribArray vao, Shader *shader, Mesh mesh){
	// Extend buffer by 1 instance
	RendererInstance *tmp_buffer = realloc(instance_buffer, sizeof(RendererInstance) * (num_instances + 1));
	if(tmp_buffer != NULL){
		instance_buffer = tmp_buffer;

		// Copy all textures to texture buffer
		for(int i = 0; i < num_textures_used; i++){
			instance_buffer[num_instances].texture[i] = textures[i].gl_tex;
		}

		// Set instance data
		instance_buffer[num_instances].num_textures_used = num_textures_used;
		instance_buffer[num_instances].vao = vao;
		if(shader->is_loaded){
			instance_buffer[num_instances].shader = shader;
		}else{
			instance_buffer[num_instances].shader = 0;
		}
		instance_buffer[num_instances].mesh = mesh;
		instance_buffer[num_instances].count = 0;

		// Initialize instance vbo
		instance_buffer[num_instances].buffer = malloc(sizeof(float) * vao.stride);

		num_instances++;
		return num_instances - 1;
	}else{
		// DebugLog(D_WARN)
		return 0;
	}
}

unsigned int FindInstance(char num_textures_used, Texture textures[16], AttribArray vao, Shader *shader, Mesh mesh){
	// Loop through all instances to find exact match
	for(int i = 0; i < num_instances; i++){
		if(
			instance_buffer[i].vao.array_object == vao.array_object && 
			instance_buffer[i].shader == shader && 
			instance_buffer[i].mesh.data == mesh.data
		){
			
			// Loop through all textures and find number that match
			int count = 0;
			if(textures != NULL){
				for(int j = 0; j < num_textures_used; j++){
					if(instance_buffer[i].texture[j] == textures[j].gl_tex){
						count++;
					}else{
						break;
					}
				}
			}

			// Only if all textures match, the instance is the same
			if(count == num_textures_used){
				return i;
			}
		}
	}

	// If no instance match is found, create a new one
	return NewInstance(num_textures_used, textures, vao, shader, mesh);
}

void AppendInstance(AttribArray vao, float data[64], Mesh mesh, Shader *shader, char num_textures_used, Texture textures[16]){
	// Retrieve instance id for inputed configuration
	unsigned int instance = FindInstance(num_textures_used, textures, vao, shader, mesh);

	// Allocate space for new element in instance buffer
	float *tmp_buffer = realloc(instance_buffer[instance].buffer, sizeof(float) * vao.stride * (instance_buffer[instance].count + 1));
	if(tmp_buffer != NULL){
		instance_buffer[instance].buffer = tmp_buffer;

		// Copy single instance data to vbo
		int current_offset = vao.stride * instance_buffer[instance].count;
		memcpy(&instance_buffer[instance].buffer[current_offset], data, sizeof(float) * vao.stride);

		// Increment instance count
		instance_buffer[instance].count++;

		num_append_instance_calls++;
	}else{
		DebugLog(D_WARN, "Couldnt allocate space for new instance buffer");
	}
}

void EmptyRenderBuffer(){
	for(int i = 0; i < num_instances; i++){
		instance_buffer[i].count = 0;
		free(instance_buffer[i].buffer);
        instance_buffer[i].buffer = NULL;
	}
	free(instance_buffer);
	instance_buffer = malloc(sizeof(RendererInstance) * 2);
	num_instances = 0;
	num_append_instance_calls = 0;
}

void PushRender(){
	// Loop through instances
	for(int instance = 0; instance < num_instances; instance++){
		// Only bind vao if different vao is bound
		if(current_vao != instance_buffer[instance].vao.array_object){
			GLCall(glBindVertexArray(instance_buffer[instance].vao.array_object));
			current_vao = instance_buffer[instance].vao.array_object;
		}

		// Bind shader
		ShaderPassUniforms(instance_buffer[instance].shader);

		// Loop through textures
		for(int texture_slot = 0; texture_slot < instance_buffer[instance].num_textures_used; texture_slot++){

			// Only bind texture to slot if different texture is bound to slot
			if(bound_textures[texture_slot] != instance_buffer[instance].texture[texture_slot]){
				
				// Only change active texture slot if different texture slot is active
				if(current_texture_unit != texture_slot){
					GLCall(glActiveTexture(GL_TEXTURE0 + texture_slot));
					current_texture_unit = texture_slot;
				}

				GLCall(glBindTexture(GL_TEXTURE_2D, instance_buffer[instance].texture[texture_slot]));
				bound_textures[texture_slot] = instance_buffer[instance].texture[texture_slot];
			}
		}

		unsigned int instance_count = instance_buffer[instance].count;
		
		// Set 'instance_vbo' data to instance data
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, instance_buffer[instance].vao.instanced_vbo));
		GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * instance_buffer[instance].vao.stride * instance_count, instance_buffer[instance].buffer, GL_STREAM_DRAW));

		// Render instance
		glDrawElementsInstanced(GL_TRIANGLES, instance_buffer[instance].mesh.index_count, instance_buffer[instance].mesh.index_gl_type, NULL, instance_count);
	}

	// Clear all instances and reset 'instance_buffer' size
	EmptyRenderBuffer();
}