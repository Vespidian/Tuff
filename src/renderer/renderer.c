#include <stdarg.h>

#include "../global.h"
#include "../gl_context.h"
#include "../gl_utils.h"

#include "renderer.h"

unsigned int quad_vbo;
unsigned int quad_ebo;

unsigned int instanced_vbo;

float quad_vertices[] = {
    1, 0, 0,	// top right
    1, 1, 0,	// bottom right
    0, 1, 0,	// bottom left
    0, 0, 0,	// top left 
};

unsigned int quad_indices[] = {
	// Clockwise
    // 0, 1, 3,	// First triangle
    // 1, 2, 3,	// Second triangle
	// Counter Clockwise
    3, 1, 0,	// First triangle
    3, 2, 1,	// Second triangle
};

mat4 default_texture_coordinates = {
    {1.0, 1.0},
    {1.0, 0.0},
    {0.0, 0.0},
    {0.0, 1.0},
};

InstanceBuffer *instance_buffer;
unsigned int num_instances = 0;
int num_append_instance_calls = 0;

// Active opengl references
unsigned int current_shader;
unsigned int current_vao;
unsigned int bound_textures[16];
unsigned int current_texture_unit;

void RendererInit(){
	instance_buffer = malloc(sizeof(InstanceBuffer) * 2);
	GLCall(glGenBuffers(1, &quad_vbo));
	GLCall(glGenBuffers(1, &quad_ebo));
	GLCall(glGenBuffers(1, &instanced_vbo));

}

AttribArray NewVAO(int num_attribs, ...){
	// Initialize vao variables
	AttribArray vao;
	vao.num_attrib_slots = 0;
	vao.stride = 0;

	// Maximum of 16 vertex attributes
	if(num_attribs > 16){
		num_attribs = 16;
	}

	// Initialize the new vertex array object
	GLCall(glGenVertexArrays(1, &vao.array_object));
	GLCall(glBindVertexArray(vao.array_object));
	// Initialize static vertex buffer
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, quad_vbo));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW));

	// Default quad vertex positions
	GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));

    // Initialize static index buffer
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_ebo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW));

	// Setting attribute locations for 'instanced_vbo'
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, instanced_vbo));

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
	unsigned int attrib_index = 1;
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
		// For matrixes, loops through each vertex attribute, otherwise only executes once
		for(int j = 0; j < mat_type; j++){
			GLCall(glEnableVertexAttribArray(attrib_index));

			// Size is 'mat_type' for matrixes and 'attribs[i]' for normal attributes
			GLCall(glVertexAttribPointer(attrib_index, (mat_type != 1) ? mat_type : attribs[i], GL_FLOAT, GL_FALSE, vao.stride * sizeof(float), (void*)(attrib_offset * sizeof(float))));
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

unsigned int NewInstance(char num_textures_used, TextureObject textures[16], AttribArray vao, unsigned int shader){
	// Extend buffer by 1 instance
	instance_buffer = realloc(instance_buffer, sizeof(InstanceBuffer) * (num_instances + 1));

	// Copy all textures to texture buffer
	for(int i = 0; i < num_textures_used; i++){
		instance_buffer[num_instances].texture[i] = textures[i].gl_tex;
	}

	// Set instance data
	instance_buffer[num_instances].num_textures_used = num_textures_used;
	instance_buffer[num_instances].vao = vao;
	instance_buffer[num_instances].shader = shader;
	instance_buffer[num_instances].count = 0;

	// Initialize instance vbo
	instance_buffer[num_instances].buffer = malloc(sizeof(float) * vao.stride);
	return num_instances++;
}

unsigned int FindInstance(char num_textures_used, TextureObject textures[16], AttribArray vao, unsigned int shader){
	// Loop throgh all instances to find exact match
	for(int i = 0; i < num_instances; i++){
		if(instance_buffer[i].vao.array_object == vao.array_object && instance_buffer[i].shader == shader){
			
			// Loop through all textures and find number that match
			int count = 0;
			for(int j = 0; j < num_textures_used; j++){
				if(instance_buffer[i].texture[j] == textures[j].gl_tex){
					count++;
				}
			}

			// Only if all textures match, the instance is the same
			if(count == num_textures_used){
				return i;
			}
		}
	}

	// If no instance match is found, create a new one
	return NewInstance(num_textures_used, textures, vao, shader);
}

void AppendInstance(AttribArray vao, float data[64], unsigned int shader, char num_textures_used, TextureObject textures[16]){
	// Retrieve instance id for inputed configuration
	unsigned int instance = FindInstance(num_textures_used, textures, vao, shader);

	// Allocate space for new element in instance buffer
	instance_buffer[instance].buffer = realloc(instance_buffer[instance].buffer, sizeof(float) * vao.stride * (instance_buffer[instance].count + 1));

	// Copy single instance data to vbo
	int current_offset = vao.stride * instance_buffer[instance].count;
	memcpy(&instance_buffer[instance].buffer[current_offset], data, sizeof(float) * vao.stride);

	// Increment instance count
	instance_buffer[instance].count++;

	num_append_instance_calls++;
}

void EmptyRenderBuffer(){
	for(int i = 0; i < num_instances; i++){
		instance_buffer[i].count = 0;
		free(instance_buffer[i].buffer);
	}
	instance_buffer = malloc(sizeof(InstanceBuffer) * 2);
	num_instances = 0;
	num_append_instance_calls = 0;
}

void PushRender(){
	// Loop through instances
	for(int instance = 0; instance < num_instances; instance++){
		unsigned int instance_count = instance_buffer[instance].count;
		
		// Only bind vao if different vao is bound
		if(current_vao != instance_buffer[instance].vao.array_object){
			glBindVertexArray(instance_buffer[instance].vao.array_object);
			current_vao = instance_buffer[instance].vao.array_object;
		}

		// Bind shader
		SetShaderProgram(instance_buffer[instance].shader);

		// Loop through textures
		for(int texture_slot = 0; texture_slot < instance_buffer[instance].num_textures_used; texture_slot++){

			// Only bind texture to slot if different texture is bound to slot
			if(bound_textures[texture_slot] != instance_buffer[instance].texture[texture_slot]){
				
				// Only change active texture slot if different texture slot is active
				if(current_texture_unit != texture_slot){
					glActiveTexture(GL_TEXTURE0 + texture_slot);
					current_texture_unit = texture_slot;
				}
				glBindTexture(GL_TEXTURE_2D, instance_buffer[instance].texture[texture_slot]);
				bound_textures[texture_slot] = instance_buffer[instance].texture[texture_slot];
			}
		}

		// Set 'instance_vbo' data to instance data
	    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * instance_buffer[instance].vao.stride * instance_count, instance_buffer[instance].buffer, GL_STREAM_DRAW);

		// Render instance
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, instance_count);
	}

	// Clear all instances and reset 'instance_buffer' size
	EmptyRenderBuffer();
}