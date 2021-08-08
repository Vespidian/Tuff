#include "../../global.h"
#include "../../event.h"
#include "../../gl_context.h"
#include "../../gl_utils.h"
#include "../renderer.h"

#include "mask.h"

AttribArray mask_vao;
ShaderObject mask_shader;

static void WindowResize(EventData event);
TilesheetObject autotile_mask_sheet;

void InitMaskedRender(){
	mask_vao = NewVAO(5, ATTR_MAT4, ATTR_VEC4, ATTR_VEC4, ATTR_VEC4, ATTR_VEC4);

	mask_shader = LoadShaderProgram("mask.shader");
	UniformSetMat4(&mask_shader, "tex_coordinates", default_texture_coordinates);
	UniformSetInt(&mask_shader, "top_texture_s", 0);
	UniformSetInt(&mask_shader, "bottom_texture_s", 1);
	UniformSetInt(&mask_shader, "mask_texture_s", 2);

	autotile_mask_sheet = LoadTilesheet("../images/autotile_mask.png", 16, 16);
	UniformSetMat4(&mask_shader, "orthographic_projection", orthographic_projection);
	BindEvent(EV_ACCURATE, SDL_WINDOWEVENT, WindowResize);
}

void MaskedRender(TilesheetObject top_sheet, unsigned int top_index, TilesheetObject bottom_sheet, unsigned int bottom_index, unsigned int mask_index, SDL_Rect dest, int zpos, SDL_Color color){

	mat4 pos;
	glm_mat4_identity(pos);
	glm_translate(pos, (vec3){dest.x, dest.y, zpos});
	glm_scale(pos, (vec3){dest.w, dest.h, 1});

	vec4 color_vec = {color.r, color.g, color.b, color.a};
	vec4 top_src = {
		(top_index % (top_sheet.texture.w / top_sheet.tile_w)) * top_sheet.tile_w, 
		top_sheet.texture.h - (top_index / (top_sheet.texture.w / top_sheet.tile_w)) * top_sheet.tile_w - top_sheet.tile_h, 
		top_sheet.tile_w, 
		top_sheet.tile_h
	};

	vec4 bottom_src = {
		(bottom_index % (bottom_sheet.texture.w / bottom_sheet.tile_w)) * bottom_sheet.tile_w, 
		bottom_sheet.texture.h - (bottom_index / (bottom_sheet.texture.w / bottom_sheet.tile_w)) * bottom_sheet.tile_w - bottom_sheet.tile_h, 
		bottom_sheet.tile_w, 
		bottom_sheet.tile_h
	};

	vec4 mask_src = {
		(mask_index % (autotile_mask_sheet.texture.w / autotile_mask_sheet.tile_w)) * autotile_mask_sheet.tile_w, 
		autotile_mask_sheet.texture.h - ((mask_index / (autotile_mask_sheet.texture.w / autotile_mask_sheet.tile_w)) * autotile_mask_sheet.tile_w) - autotile_mask_sheet.tile_h, 
		autotile_mask_sheet.tile_w, 
		autotile_mask_sheet.tile_h
	};

	float data[64];
	memcpy(&data[0], pos, sizeof(mat4));
	memcpy(&data[16], color_vec, sizeof(vec4));
	memcpy(&data[20], top_src, sizeof(vec4));
	memcpy(&data[24], bottom_src, sizeof(vec4));
	memcpy(&data[28], mask_src, sizeof(vec4));


	TextureObject textures[16] = {top_sheet.texture, bottom_sheet.texture, autotile_mask_sheet.texture};
	AppendInstance(mask_vao, data, &mask_shader, 3, textures);
}

static void WindowResize(EventData event){
	if(event.e->window.event == SDL_WINDOWEVENT_RESIZED){
		// SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
		// glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		// glm_ortho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, -z_depth / 2, z_depth / 2, orthographic_projection);
		UniformSetMat4(&mask_shader, "orthographic_projection", orthographic_projection);
	}
}