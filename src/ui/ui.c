#include "../global.h"
#include "../debug.h"
#include "../gl_utils.h"
#include "../renderer/quad.h"
#include "../renderer/renderer.h"

//UI Elements
#include "resizable_rect.h"

#include "ui.h"
#include "../gl_context.h"

bool ui_hovered = false;
bool ui_selected = false;
TilesheetObject ui_tilesheet;

ShaderObject ui_shader;
AttribArray ui_vao;


void InitUI(){
	ui_tilesheet = LoadTilesheet("../images/ui/ui.png", 16, 16);


	ui_vao = NewVAO(5, ATTR_MAT4, ATTR_VEC4, ATTR_VEC4, ATTR_VEC4, ATTR_VEC4);
	ui_shader = LoadShaderProgram("ui.shader");
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	glUniformBlockBinding(ui_shader.id, glGetUniformBlockIndex(ui_shader.id, "ShaderGlobals"), 0);
	UniformSetMat4(&ui_shader, "tex_coordinates", default_texture_coordinates);


	DebugLog(D_ACT, "Initialized UI subsystem");
}

void CalculateRadiusLimit(float length, float *r1, float *r2){
	if(length - *r1 - *r2 < 0){
		float scale_factor = length / (*r1 + *r2);
		*r1 *= scale_factor;
		*r2 *= scale_factor;
	}
	if(*r1 * 2 > length){
		*r1 = length * 0.5f;
	}
	if(*r2 * 2 > length){
		*r2 = length * 0.5f;
	}
}

void DrawUIElement(){
	/*
	mat3 model_a;
	vec4 border_a;
	vec4 radius_a;
	vec4 color_a;
	vec4 border_color_a;
	*/
	// float border_v = 0;
	// float radius_v = 0;
	// Vector4 transform = {100, 100, (cos(SDL_GetTicks() / 1000.0) + 1)/2 * 100 + 20, (sin(SDL_GetTicks() / 1000.0) + 1)/2 * 100 + 20};
	Vector4 transform = {300, 300, (cos(SDL_GetTicks() / 1000.0) + 1)/2 * 100 + 100, (sin(SDL_GetTicks() / 1000.0) + 1)/2 * 100 + 100};


	// Vector4 transform = {100, 100, 600, (sin(SDL_GetTicks() / 1000.0) + 1)/2 * 100 + 100};
	// Vector4 transform = {100, 100, (sin(SDL_GetTicks() / 1000.0) + 1)/2 * 600 + 100, 600};
	// Vector4 transform = {100, 100, 600, 100};
	// Vector4 transform = {100, 100, 100, 100};
	mat4 matrix;
	glm_mat4_identity(matrix);
	glm_translate(matrix, (vec3){transform.x, transform.y, 100});
	glm_scale(matrix, (vec2){transform.z, transform.w});

	// vec4 border = {border_v, border_v, border_v, border_v};
	// vec4 radius = {radius_v, radius_v, radius_v, radius_v};
	Vector4 border = {1, 1, 1, 1};
	// Vector4 radius = {90, 90, 90, 90};
	// Vector4 radius = {30, 0, 30, 0};
	// Vector4 radius = {90, 200, 110, 90};
	Vector4 radius = {180, 180, 0, 0};
	// Vector4 radius = {0, 0, 0, 0};
	// Vector4 radius = {0, 0, 180, 90};
	Vector4 color = {1, 0.5, 0.25, 1};
	Vector4 border_color = {1, 1, 1, 1};


	/**
	 * Limiting corner radiuses
	 * 4 ifs needed:
	 * x against y
	 * x against w
	 * 
	 * z against y
	 * z against w
	 */
	
	// float diagonal = glm_vec2_distance2(rx, rz);
	// printf("diagonal: %f\n", diagonal);
	// if(diagonal < 0){
	// }
	// if(transform.z - radius.x - radius.y < 0 && transform.w - radius.x - radius.w){
	// 	vec2 rx = {radius.x, radius.x};
	// 	vec2 rz = {transform.z - radius.z, transform.w - radius.z};

	// }
	CalculateRadiusLimit(transform.z, &radius.z, &radius.w);
	CalculateRadiusLimit(transform.w, &radius.y, &radius.z);
	CalculateRadiusLimit(transform.w, &radius.w, &radius.x);
	CalculateRadiusLimit(transform.z, &radius.x, &radius.y);
	// z = width
	// w = height

	float data[64];
	memcpy(&data[0], matrix, sizeof(mat4));
	memcpy(&data[16], border.v, sizeof(vec4));
	memcpy(&data[20], radius.v, sizeof(vec4));
	memcpy(&data[24], color.v, sizeof(vec4));
	memcpy(&data[28], border_color.v, sizeof(vec4));

	TextureObject texture_array[16] = {1};
	AppendInstance(ui_vao, data, &ui_shader, 1, texture_array);
}

long IntegerLerp(long start, long end, uint16_t amount){
	long lerpLimit = 1;
	long velocity = (end - start) * ((float)amount / 65535.0f);
	if(velocity < lerpLimit && start + lerpLimit <= end){
		return lerpLimit;
	}
	return velocity;
}

SDL_Rect VerticalRectList(int num_items, int itemIndex, Vector2 size, Vector2 origin, int spacing){
	SDL_Rect rect = {0, 0, size.x, size.y};
	int yOffset = 0;
	if(num_items % 2 == 0){//Even number of elements
		rect.x = origin.x - size.x / 2;
		if(itemIndex < num_items / 2){//Top half
			yOffset = (num_items / 2 - itemIndex);
			rect.y = origin.y - (spacing / 2 + size.y * yOffset + spacing * (yOffset - 1));
		}else{//Bottom half
			yOffset = (itemIndex - num_items / 2);
			rect.y = origin.y + (spacing / 2 + size.y * yOffset + spacing * yOffset);
		}
	}else{//Odd number of elements
		rect.x = origin.x - size.x / 2;
		if(itemIndex < num_items / 2){//top half
			yOffset = (num_items / 2 - itemIndex);
			rect.y = origin.y - (size.y / 2 + size.y * yOffset + spacing * yOffset);
		}else if(itemIndex == (num_items + 1) / 2 - 1){//Middle element
			rect.y = origin.y - (size.y / 2);
		}else{//Bottom half
			yOffset = (itemIndex - 1 - num_items / 2);
			rect.y = origin.y + (size.y / 2 + size.y * yOffset + spacing * (yOffset	+ 1));
		}
	}
	return rect;
}

Vector4_i VerticalRectList_vec(int num_items, int itemIndex, Vector2 size, Vector2 origin, int spacing){
	SDL_Rect tmp = VerticalRectList(num_items, itemIndex, size, origin, spacing);
	return (Vector4_i){tmp.x, tmp.y, tmp.w, tmp.h};
}

void DrawVListBackground(int num_items, Vector2 size, Vector2 origin, int spacing, uint8_t opacity){
	int padding = 8;
	SDL_Rect rect;
	rect.w = size.x + padding * 2;
	rect.h = num_items * size.y + (num_items + 1) * padding;
	rect.x = SCREEN_WIDTH / 2 - rect.w / 2;
	rect.y = SCREEN_HEIGHT / 2 - rect.h / 2;
	RenderTilesheet(ui_tilesheet, 0, &rect, RNDR_UI, (Vector4){1, 1, 1, 1});
}