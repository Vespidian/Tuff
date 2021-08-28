#include "../global.h"
#include "../debug.h"
#include "../gl_utils.h"
#include "../renderer/quad.h"
#include "../renderer/renderer.h"

//UI Elements

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

void RenderUIElement(UIElement *element){
	if(element->text != NULL){
		RenderTextEx(
			element->font, 
			element->text_size, 
			10,
			// element->padding_calculated.w + element->border_calculated.w + element->transform.x,
			element->transform.w / 2,
			// 0,
			// 0,
			element->text_color,
			TEXT_ALIGN_LEFT,
			-1,
			element->text
		);
	}
	/*
	mat3 model_a;
	vec4 border_a;
	vec4 radius_a;
	vec4 color_a;
	vec4 border_color_a;
	*/
	Vector4 transform = element->transform_absolute;
	mat4 matrix;
	glm_mat4_identity(matrix);
	glm_translate(matrix, (vec3){transform.x, transform.y, 100});
	glm_scale(matrix, (vec2){transform.z, transform.w});

	Vector4 radius = element->radius;
	/**
	 * Limiting corner radiuses
	 * 4 ifs needed:
	 * x against y
	 * x against w
	 * 
	 * z against y
	 * z against w
	 */
	CalculateRadiusLimit(transform.z, &radius.z, &radius.w);
	CalculateRadiusLimit(transform.w, &radius.y, &radius.z);
	CalculateRadiusLimit(transform.w, &radius.w, &radius.x);
	CalculateRadiusLimit(transform.z, &radius.x, &radius.y);

	float data[64];
	memcpy(&data[0], matrix, sizeof(mat4));
	memcpy(&data[16], element->border_calculated.v, sizeof(vec4));
	memcpy(&data[20], radius.v, sizeof(vec4));
	memcpy(&data[24], element->color.v, sizeof(vec4));
	memcpy(&data[28], element->border_color.v, sizeof(vec4));

	TextureObject texture_array[16] = {1};
	AppendInstance(ui_vao, data, &ui_shader, 1, texture_array);
}

UIClass *uiNewClass(){
	return NULL;
}

UIElement *uiNewElement(UIElement *parent){
	// parent->children[parent->num_children] = realloc(parent->children[parent->num_children], sizeof(UIElement) * ++parent->num_children);
	parent->children = realloc(parent->children, sizeof(UIElement) * ++parent->num_children);
	parent->children[parent->num_children - 1].parent = parent;
	return &parent->children[parent->num_children - 1];
}

Vector4 CalculatePadding(UIElement *element){
	int axis = 0; // Axis (0 = x axis, 1 = y axis)
	Vector4 padding;
	for(int i = 0; i < 4; i++){
		axis = i % 2;
		switch(element->padding_type.v[i]){
			case UI_PERCENT:
				padding.v[i] = element->parent->transform.v[axis + 2] * element->padding.v[i];
				break;
			case UI_STYLE_INHERIT:
				padding.v[i] = element->parent->padding.v[i];
				break;
			default:
				padding.v[i] = element->padding.v[i];
				break;
		}
	}
	element->padding_calculated = padding;
	return padding;
}

Vector4 CalculateBorder(UIElement *element){
	int axis = 0; // Axis (0 = x axis, 1 = y axis)
	Vector4 border;
	for(int i = 0; i < 4; i++){
		axis = i % 2;
		switch(element->border_type.v[i]){
			case UI_PERCENT:
				border.v[i] = element->parent->transform.v[axis + 2] * element->border.v[i];
				// printf("%f\n", element->parent->transform.v[axis + 2]);
				printf("%f\n", border.v[i]);
				break;
			case UI_STYLE_INHERIT:
				border.v[i] = element->parent->border.v[i];
				break;
			default:
				border.v[i] = element->border.v[i];
				break;
		}
	}
	element->border_calculated = border;
	return border;
}

Vector2 CalculateScale(UIElement *element){
	Vector2 scale;
	Vector4 border = CalculateBorder(element);
	Vector4 padding = CalculatePadding(element);
	for(int i = 2; i <= 3; i++){
		switch(element->transform_type.v[i]){
			case UI_PERCENT:
				scale.v[i - 2] = element->parent->transform.v[i] * element->transform.v[i];
				break;
			default:
				scale.v[i - 2] = element->transform.v[i];
				break;
		}
		if(i == 2){
			scale.v[i - 2] += (border.v[i - 1] + border.v[i + 1]) + (padding.v[i - 1] + padding.v[i + 1]);
		}else{
			scale.v[i - 2] += (border.v[i - 1] + border.v[i - 3]) + (padding.v[i - 1] + padding.v[i - 3]);
		}
	}
	element->transform_calculated.z = scale.x;
	element->transform_calculated.w = scale.y;
	return scale;
}

Vector4 CalculateMargin(UIElement *element){
	int axis = 0; // Axis (0 = x axis, 1 = y axis)
	Vector4 margin;
	for(int i = 0; i < 4; i++){
		axis = i % 2;
		switch(element->margin_type.v[i]){
			case UI_PERCENT:
				margin.v[i] = element->parent->transform.v[axis + 2] * element->margin.v[i];
				break;
			case UI_STYLE_INHERIT:
				margin.v[i] = element->parent->margin.v[i];
				break;
			default:
				margin.v[i] = element->margin.v[i];
				break;
		}
	}
	element->margin_calculated = margin;
	return margin;
}

Vector2 CalculatePosition_relative(UIElement *element){
	Vector2 position;
	Vector4 margin = CalculateMargin(element);
	for(int i = 0; i <= 1; i++){
		switch(element->transform_type.v[i]){
			case UI_PERCENT:
				position.v[i] = element->transform.v[i + 2] * element->transform.v[i];
				break;
			case UI_TRANSFORM_PIXELS_INVERTED:
				position.v[i] = element->parent->transform.v[i + 2] - element->transform.v[i];
				break;
			case UI_TRANSFORM_ABSOLUTE:
				position.v[i] = element->transform.v[i];
				break;
			default:
				position.v[i] = element->transform.v[i];
				break;
		}

		// Add margin to position
		if(element->transform_type.v[i] == UI_TRANSFORM_PIXELS_INVERTED){ // bottom / right
			position.v[i] += -margin.v[i + 1];
		}else{ // top / left
			position.v[i] += margin.v[!i * 3];
		}
	}
	return position;
}

UI_OriginType_et CalculateOriginType(UIElement *element){
	UI_OriginType_et origin = element->origin;
	if(!element->origin_set){
		bool o1 = element->transform_type.x && UI_TRANSFORM_PIXELS_INVERTED;
		bool o2 = element->transform_type.y && UI_TRANSFORM_PIXELS_INVERTED;

		if(o1 && o2){
			origin = UI_ORIGIN_BOTTOM_RIGHT;
		}else if(o1){
			origin = UI_ORIGIN_TOP_RIGHT;
		}else if(o2){
			origin = UI_ORIGIN_BOTTOM_LEFT;
		}
	}
	return origin;
}

Vector2 CalculateOriginOffset(UIElement *element){
	Vector2 origin_offset = {0, 0};
	UI_OriginType_et origin = CalculateOriginType(element);
	Vector2 scale = CalculateScale(element);
	switch(origin){
		default: // UI_ORIGIN_TOP_LEFT
			// By default the origin is in the top left corner, so no offset
			break;
		case UI_ORIGIN_TOP_MIDDLE:
			origin_offset = (Vector2){scale.x / 2, 0};
			break;
		case UI_ORIGIN_TOP_RIGHT:
			origin_offset = (Vector2){scale.x, 0};
			break;

		case UI_ORIGIN_CENTER:
			origin_offset = (Vector2){scale.x / 2, scale.y / 2};
			break;
		case UI_ORIGIN_CENTER_LEFT:
			origin_offset = (Vector2){0, scale.y / 2};
			break;
		case UI_ORIGIN_CENTER_RIGHT:
			origin_offset = (Vector2){scale.x, scale.y / 2};
			break;

		case UI_ORIGIN_BOTTOM_LEFT:
			origin_offset = (Vector2){0, scale.y};
			break;
		case UI_ORIGIN_BOTTOM_MIDDLE:
			origin_offset = (Vector2){scale.x / 2, scale.y};
			break;
		case UI_ORIGIN_BOTTOM_RIGHT:
			origin_offset = (Vector2){scale.x, scale.y};
			break;
	}
	return origin_offset;
}

// Vector4 CalculateCoverage(UIElement *element){

// }

void CalculatePropertyPixelValue(UIElement *element){
	Vector2 scale = CalculateScale(element);

	Vector2 position = CalculatePosition_relative(element);

	Vector2 origin_offset = CalculateOriginOffset(element);
	// Vector2 origin_offset = {0, 0};
	

	// if(element->parent->align == UI_ALIGN_VERTICAL){
	// 	axis = 1;
	// }else{
	// 	axis = 0;
	// }

	position.x -= origin_offset.x;
	position.y -= origin_offset.y;
	element->transform_calculated.x = position.x;
	element->transform_calculated.y = position.y;


	element->transform_absolute.x = element->parent->transform_absolute.x + position.x;
	element->transform_absolute.y = element->parent->transform_absolute.y + position.y;
	element->transform_absolute.z = scale.x;
	element->transform_absolute.w = scale.y;
}

void CalculateChildren(UIElement *element){
	CalculatePropertyPixelValue(element);

	if(element->children != NULL){
		for(int i = 0; i < element->num_children; i++){
			CalculateChildren(&element->children[i]);
		}
	}
}

void RenderChildren(UIElement *element){
	if(element->is_active){
		RenderUIElement(element);
	}

	if(element->children != NULL){
		for(int i = 0; i < element->num_children; i++){
			RenderChildren(&element->children[i]);
		}
	}
}

void RenderUIInstance(UIScene *scene){
	// if(scene->needs_update){
	// 	CalculateChildren(&scene->body);
	// }
	for(int i = 0; i < scene->body.num_children; i++){
		CalculateChildren(&scene->body.children[i]);
	}

	for(int i = 0; i < scene->body.num_children; i++){
		RenderChildren(&scene->body.children[i]);
	}
}

UIScene *uiLoadFile(UIScene *scene){
	
	scene->num_classes = 0;

	scene->dynamic = false; // Scenes are static by default

	scene->needs_update = true;

	scene->body.name = malloc(sizeof(char) * 5);
	strcpy(scene->body.name, "body");

	scene->body.transform = (Vector4){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	scene->body.transform_type = (Vector4_i){UI_PIXELS, UI_PIXELS, UI_PIXELS, UI_PIXELS};
	
	scene->body.color = (Vector4){1, 0.5, 0.75, 1};
	scene->body.is_active = true;
	scene->body.parent = NULL;
	scene->body.num_children = 0;



	UIElement *test = uiNewElement(&scene->body);
	test->children = NULL;
	test->parent = &scene->body;

	test->transform = (Vector4){20, 20, 0.5, 500};
	test->transform_type = (Vector4_i){UI_TRANSFORM_PIXELS_INVERTED, UI_PIXELS, UI_PERCENT, UI_PIXELS};
	
	test->margin = (Vector4){80, 0, 0, 0};
	test->margin_type = (Vector4_i){UI_PIXELS, UI_PIXELS, UI_PIXELS, UI_PIXELS};
	
	test->padding = (Vector4){0, 0, 0, 0};
	test->padding_type = (Vector4_i){UI_PIXELS, UI_PIXELS, UI_PIXELS, UI_PIXELS};
	
	test->border = (Vector4){2, 0.1, 0, 0};
	test->border_type = (Vector4_i){UI_PIXELS, UI_PERCENT, UI_PIXELS, UI_PIXELS};
	
	test->radius = (Vector4){50, 5, 50, 10};
	test->radius_type = (Vector4_i){UI_PIXELS, UI_PIXELS, UI_PIXELS, UI_PIXELS};

	test->color = (Vector4){0, 1, 0.5, 1};
	test->border_color = (Vector4){0, 0, 0.5, 1};

	test->origin = UI_ORIGIN_TOP_LEFT;
	// test->origin_set = true;

	test->is_active = true;

	test->font = &default_font;
	test->text_size = 1;
	test->text = malloc(sizeof(char) * 6);
	strcpy(test->text, "test;");
	test->text[5] = 0;
	test->text_color = (Vector4){1, 1, 1, 1};
	// test->text = NULL;



	for(int i = 0; i < scene->body.num_children; i++){
		CalculateChildren(&scene->body.children[i]);
	}

	return NULL;
}