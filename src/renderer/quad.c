#include "../global.h"
#include "../gl_context.h"
// #include "../gl_utils.h"
#include "../shader.h"
#include "../event.h"
#include "renderer.h"

#include "quad.h"

Shader quad_shader;

AttribArray quad_vao;

void SetQuadProjection();

void InitQuadRender(){

	quad_vao = NewVAO(3, ATTR_MAT4, ATTR_VEC4, ATTR_VEC4);
    quad_shader = ShaderOpen("shaders/quad_default.shader");
	ShaderUniformSetSampler2D(&quad_shader, "src_texture", 0);
	ShaderUniformSetMat4(&quad_shader, "tex_coordinates", default_texture_coordinates);
    ShaderUniformSetMat4(&quad_shader, "projection", orthographic_projection);

	BindEvent(EV_POLL_ACCURATE, SDL_WINDOWEVENT, SetQuadProjection);
}

void SetQuadProjection(){
    ShaderUniformSetMat4(&quad_shader, "projection", orthographic_projection);
}

void RenderQuad(Texture texture, SDL_Rect *src, SDL_Rect *dst, int zpos, Vector4 color, float rot){
	// NULL to fill entire viewport
	if(dst == NULL){
		// dst = &(SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
		dst->x = 0;
		dst->y = 0;
		dst->w = SCREEN_WIDTH;
		dst->h = SCREEN_HEIGHT;
	}
	// NULL for entire texture
	if(src == NULL){
		// src = &(SDL_Rect){0, 0, texture.w, texture.h};
		src->x = 0;
		src->y = 0;
		src->w = texture.w;
		src->h = texture.h;
	}
		
	// Populate model matrix
	mat4 model_matrix;
	glm_mat4_identity(model_matrix);
	glm_translate(model_matrix, (vec3){dst->x, dst->y, zpos});
	glm_rotate_at(model_matrix, (vec3){dst->w / 2.0, dst->h / 2.0, 0}, rot, (vec3){0, 0, 1});
	glm_scale(model_matrix, (vec3){dst->w, dst->h, 1});

	// Convert color and texture to vec4 for easy memcpy
	vec4 color_vec = {color.r, color.g, color.b, color.a};
	vec4 texture_src = {
		src->x, 
		texture.h - src->y - src->h, 
		src->w, 
		src->h
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
	AppendInstance(quad_vao, data, &quad_shader, 1, texture_array);

}
// {				Model Matrix				 } {  Color  } { Texture source positions }
// _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, r, g, b, a, x, y, w, h


void RenderTilesheet(TilesheetObject tilesheet, unsigned int index, SDL_Rect *dst, int zpos, Vector4 color){
	// NULL to fill entire viewport
	if(dst == NULL){
		// dst = &(SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
		dst->x = 0;
		dst->y = 0;
		dst->w = SCREEN_WIDTH;
		dst->h = SCREEN_HEIGHT;
	}

	mat4 pos;
	glm_mat4_identity(pos);
	glm_translate(pos, (vec3){dst->x, dst->y, zpos});
	glm_scale(pos, (vec3){dst->w, dst->h, 1});

	vec4 color_vec = {color.r, color.g, color.b, color.a};
	vec4 texture_src = {
		(index % (tilesheet.texture.w / tilesheet.tile_w)) * tilesheet.tile_w, 
		tilesheet.texture.h - ((index / (tilesheet.texture.w / tilesheet.tile_w)) * tilesheet.tile_w) - tilesheet.tile_h, 
		tilesheet.tile_w, 
		tilesheet.tile_h
	};

	float data[64];
	memcpy(&data[0], pos, sizeof(mat4));
	memcpy(&data[16], color_vec, sizeof(vec4));
	memcpy(&data[20], texture_src, sizeof(vec4));



	Texture textures[16] = {tilesheet.texture};
	AppendInstance(quad_vao, data, &quad_shader, 1, textures);
}