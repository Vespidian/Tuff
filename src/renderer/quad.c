#include <stdbool.h>
#include <cglm/cglm.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "../sdl_gl_init.h"

#include "../engine.h"
#include "../shader.h"
#include "../textures.h"
#include "../event.h"
#include "../gltf.h"
#include "renderer.h"

#include "quad.h"

Shader quad_shader;

AttribArray quad_vao;
GLTF quad_gltf;
Mesh quad_mesh;

// void SetQuadProjection();

mat4 default_texture_coordinates = {
    {0.0, 1.0},
    {0.0, 0.0},
    {1.0, 1.0},
    {1.0, 0.0},
};

static void InitQuadGL(){
	quad_gltf = GLTFOpen("../assets/models/plane.gltf");
	quad_mesh = quad_gltf.meshes[0];

	if(!quad_mesh.gl_data.is_loaded){
		// set up 'model.renderer' (vao and vbo)
		glGenVertexArrays(1, &quad_mesh.gl_data.vao);
		glBindVertexArray(quad_mesh.gl_data.vao);

		glGenBuffers(1, &quad_mesh.gl_data.pos_vbo);
		glGenBuffers(1, &quad_mesh.gl_data.ebo);

		// Position
		if(quad_mesh.position_exists){
			glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.gl_data.pos_vbo);
			glBufferData(GL_ARRAY_BUFFER, quad_mesh.position_bytelength, quad_mesh.data + quad_mesh.position_offset, GL_STATIC_DRAW);
			// glEnableVertexAttribArray(0);
			glEnableVertexArrayAttrib(quad_mesh.gl_data.vao, 0);
			glVertexAttribPointer(0, quad_mesh.position_size, quad_mesh.position_gl_type, GL_FALSE, 0, (void*)(0));
		}

		if(quad_mesh.index_exists){
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_mesh.gl_data.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad_mesh.index_bytelength, quad_mesh.data + quad_mesh.index_offset, GL_STATIC_DRAW);
		}

		quad_mesh.gl_data.is_loaded = true;
	}

	quad_vao = NewVAO(quad_mesh.gl_data.vao, 1, 3, ATTR_MAT4, ATTR_VEC4, ATTR_VEC4);
}

void InitQuadRender(){
	InitQuadGL();

    quad_shader = ShaderOpen("../assets/shaders/quad_default.shader");
	ShaderUniformSetSampler2D(&quad_shader, "src_texture", 0);
	ShaderUniformSetMat4(&quad_shader, "tex_coordinates", default_texture_coordinates);

	// Setup global uniforms
	glUniformBlockBinding(quad_shader.id, glGetUniformBlockIndex(quad_shader.id, "ShaderGlobals"), 0);

}

void RenderQuad(Texture texture, Vector4 *src, Vector4 *dst, int zpos, Vector4 color, float rot){
	Vector4 dstt;
	// NULL to fill entire viewport
	if(dst == NULL){
		// dst = &(SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
		dstt.x = 0;
		dstt.y = 0;
		dstt.z = SCREEN_WIDTH;
		dstt.w = SCREEN_HEIGHT;
	}else{
		dstt.x = dst->x;
		dstt.y = dst->y;
		dstt.z = dst->z;
		dstt.w = dst->w;
	}
	Vector4 rectsrc = {0, 0, 0, 0};
	// NULL for entire texture
	if(src == NULL){
		// src = &(SDL_Rect){0, 0, texture.w, texture.h};
		rectsrc.x = 0;
		rectsrc.y = 0;
		rectsrc.z = texture.w;
		rectsrc.w = texture.h;
	}else{
		rectsrc = *src;
	}
		
	// Populate model matrix
	mat4 model_matrix;
	glm_mat4_identity(model_matrix);
	glm_translate(model_matrix, (vec3){dstt.x, dstt.y, zpos});
	glm_rotate_at(model_matrix, (vec3){dstt.z / 2.0, dstt.w / 2.0, 0}, rot, (vec3){0, 0, 1});
	glm_scale(model_matrix, (vec3){dstt.z, dstt.w, 1});

	// Convert color and texture to vec4 for easy memcpy
	vec4 color_vec = {color.r, color.g, color.b, color.a};
	vec4 texture_src = {
		rectsrc.x, 
		texture.h - rectsrc.y - rectsrc.w, 
		rectsrc.z, 
		rectsrc.w
		// 16
	};
	// vec4 texture_src = {
	// 	0, 0, 16, 16
	// };


	// Copy all data into data array
	float data[64];
	memcpy(&data[0], model_matrix, sizeof(mat4));
	memcpy(&data[16], color_vec, sizeof(vec4));
	memcpy(&data[20], texture_src, sizeof(vec4));

	// Put texture in array
	Texture texture_array[16] = {texture};

	//Send data to be processed
	AppendInstance(quad_vao, data, quad_mesh, &quad_shader, 1, texture_array);
	// #include "../material.h"
	// #include "../gltf.h"
	// #include "../bundle.h"
	// #include "../scene.h"
	// extern Model model;
	// // extern Material mat;
	// AppendInstance(model.attr, data, quad_mesh, &quad_shader, 1, texture_array);

}
// {				Model Matrix				 } {  Color  } { Texture source positions }
// _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, r, g, b, a, 			x, y, w, h


void RenderTilesheet(Texture texture, unsigned int index, iVector2 tile_size, Vector4 *dst, int zpos, Vector4 color){
	// NULL to fill entire viewport
	if(dst == NULL){
		// dst = &(SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
		dst->x = 0;
		dst->y = 0;
		dst->z = SCREEN_WIDTH;
		dst->w = SCREEN_HEIGHT;
	}

	mat4 pos;
	glm_mat4_identity(pos);
	glm_translate(pos, (vec3){dst->x, dst->y, zpos});
	glm_scale(pos, (vec3){dst->z, dst->w, 1});

	vec4 color_vec = {color.r, color.g, color.b, color.a};
	vec4 texture_src = {
		(index % (texture.w / tile_size.x)) * tile_size.x, 
		texture.h - ((index / (texture.w / tile_size.x)) * tile_size.x) - tile_size.y, 
		tile_size.x, 
		tile_size.y
	};

	float data[64];
	memcpy(&data[0], pos, sizeof(mat4));
	memcpy(&data[16], color_vec, sizeof(vec4));
	memcpy(&data[20], texture_src, sizeof(vec4));



	Texture textures[16] = {texture};
	AppendInstance(quad_vao, data, quad_mesh, &quad_shader, 1, textures);
}