#include "../global.h"
#include "../debug.h"
#include "../gl_utils.h"
#include "../renderer/quad.h"
#include "../renderer/renderer.h"

//UI Elements
#include "../gl_context.h"

#include "ui_interact.h"
#include "ui_layout.h"

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
	Vector4 transform = element->transform;
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
	memcpy(&data[16], element->border.v, sizeof(vec4));
	memcpy(&data[20], radius.v, sizeof(vec4));
	memcpy(&data[24], element->color.v, sizeof(vec4));
	memcpy(&data[28], element->border_color.v, sizeof(vec4));

	TextureObject texture_array[16] = {1};
	AppendInstance(ui_vao, data, &ui_shader, 1, texture_array);
}

UIClass *UI_NewClass(UIScene *scene){
	scene->classes = realloc(scene->classes, sizeof(UIClass) * (scene->num_classes + 1));

	UIClass *class = &scene->classes[scene->num_classes];

	class->border_color_defined = false;
	class->color_defined = false;
	class->font_defined = false;
	class->text_color_defined = false;
	class->text_size_defined = false;
	class->transition_defined = false;
	class->origin = UI_ORIGIN_TOP_LEFT;

	class->transform_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->transform_relative_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->margin_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->border_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->padding_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->radius_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};


	return &scene->classes[scene->num_classes++];
}

UIElement *UI_NewElement(UIElement *parent){
	// parent->children[parent->num_children] = realloc(parent->children[parent->num_children], sizeof(UIElement) * ++parent->num_children);
	parent->children = realloc(parent->children, sizeof(UIElement) * ++parent->num_children);
	parent->children[parent->num_children - 1].parent = parent;
	return &parent->children[parent->num_children - 1];
}

static Vector4 CalculatePadding(UIElement *element, UIClass *class){
	int axis = 0; // Axis (0 = x axis, 1 = y axis)
	Vector4 padding;
	for(int i = 0; i < 4; i++){
		axis = i % 2;
		bool type_defined = true;
		switch(class->padding_type.v[i]){
			case UI_PERCENT:
				padding.v[i] = element->parent->transform.v[axis + 2] * class->padding.v[i];
				break;
			case UI_STYLE_INHERIT:
				padding.v[i] = element->parent->padding.v[i];
				break;
			case UI_PIXELS:
				padding.v[i] = class->padding.v[i];
				break;
			default: // UI_UNDEFINED or anything not supported by this property
				type_defined = false;
				break;
		}
		if(type_defined){
			element->padding.v[i] = padding.v[i];
		}
	}
	return padding;
}

static Vector4 CalculateBorder(UIElement *element, UIClass *class){
	int axis = 0; // Axis (0 = x axis, 1 = y axis)
	Vector4 border;
	for(int i = 0; i < 4; i++){
		axis = i % 2;
		bool type_defined = true;
		switch(class->border_type.v[i]){
			case UI_PERCENT:
				border.v[i] = element->parent->transform.v[axis + 2] * class->border.v[i];
				break;
			case UI_STYLE_INHERIT:
				border.v[i] = element->parent->border.v[i];
				break;
			case UI_PIXELS:
				border.v[i] = class->border.v[i];
				break;
			default: // UI_UNDEFINED or anything not supported by this property
				type_defined = false;
				break;
		}
		if(type_defined){
			element->border.v[i] = border.v[i];
		}
	}
	return border;
}

static Vector2 CalculateScale(UIElement *element, UIClass *class){
	Vector2 scale;
	// Vector4 border = CalculateBorder(element, element->border_type, element->border);
	// Vector4 padding = CalculatePadding(element, element->padding_type, element->padding);
	Vector4 border = element->border;
	Vector4 padding = element->padding;
	for(int i = 2; i <= 3; i++){
		bool type_defined = true;
		switch(class->transform_type.v[i]){
			case UI_PERCENT:
				scale.v[i - 2] = element->parent->transform.v[i] * class->transform.v[i];
				break;
			case UI_PIXELS:
				scale.v[i - 2] = class->transform.v[i];
				break;
			default: // UI_UNDEFINED or anything not supported by this property
				type_defined = false;
				break;
		}
		if(i == 2){
			scale.v[i - 2] += (border.v[i - 1] + border.v[i + 1]) + (padding.v[i - 1] + padding.v[i + 1]);
		}else{
			scale.v[i - 2] += (border.v[i - 1] + border.v[i - 3]) + (padding.v[i - 1] + padding.v[i - 3]);
		}
		if(type_defined){
			element->transform.v[i] = scale.v[i - 2];
		}
	}
	return scale;
}

static Vector4 CalculateMargin(UIElement *element, UIClass *class){
	int axis = 0; // Axis (0 = x axis, 1 = y axis)
	Vector4 margin;
	for(int i = 0; i < 4; i++){
		axis = i % 2;
		bool type_defined = true;
		switch(class->margin_type.v[i]){
			case UI_PERCENT:
				margin.v[i] = element->parent->transform.v[axis + 2] * class->margin.v[i];
				break;
			case UI_STYLE_INHERIT:
				margin.v[i] = element->parent->margin.v[i];
				break;
			case UI_PIXELS:
				margin.v[i] = class->margin.v[i];
				break;
			default: // UI_UNDEFINED or anything not supported by this property
				type_defined = false;
				break;
		}
		if(type_defined){
			element->margin.v[i] = margin.v[i];
		}
	}
	return margin;
}

static Vector2 CalculatePosition_relative(UIElement *element, UIClass *class){
	Vector2 position;
	// Vector4 margin = CalculateMargin(element, element->margin_type, element->margin);
	Vector4 margin = element->margin;
	for(int i = 0; i <= 1; i++){
		bool type_defined = true;
		switch(class->transform_type.v[i]){
			case UI_PERCENT:
				position.v[i] = element->parent->transform.v[i + 2] * class->transform.v[i];
				break;
			case UI_TRANSFORM_PIXELS_INVERTED:
				position.v[i] = element->parent->transform.v[i + 2] - class->transform.v[i];
				break;
			case UI_TRANSFORM_ABSOLUTE:
				position.v[i] = class->transform.v[i];
				break;
			case UI_PIXELS:
				position.v[i] = class->transform.v[i];
				break;
			default: // UI_UNDEFINED or anything not supported by this property
				type_defined = false;
				break;
		}

		// Add margin to position
		if(class->transform_type.v[i] == UI_TRANSFORM_PIXELS_INVERTED){ // bottom / right
			position.v[i] += -margin.v[i + 1];
		}else if(class->transform_type.v[i] != UI_PERCENT){ // top / left
			position.v[i] += margin.v[!i * 3];
		}

		if(type_defined){
			element->transform.v[i] = position.v[i];
		}
	}
	return position;
}

static UI_OriginType CalculateOriginType(UIElement *element, UIClass *class){
	UI_OriginType origin = class->origin;
	if(origin != UI_ORIGIN_TOP_LEFT){
		bool o1 = class->transform_type.x && UI_TRANSFORM_PIXELS_INVERTED;
		bool o2 = class->transform_type.y && UI_TRANSFORM_PIXELS_INVERTED;

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

static Vector2 CalculateOriginOffset(UIElement *element, UIClass *class){
	Vector2 origin_offset = {0, 0};
	UI_OriginType origin = CalculateOriginType(element, class);
	// Vector2 scale = CalculateScale(element, element->transform_type, element->transform);
	Vector2 scale = (Vector2){element->transform.v[2], element->transform.v[3]};
	switch(origin){
		default: // UI_ORIGIN_TOP_LEFT or UI_UNDEFINED
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
	element->transform.x -= origin_offset.x;
	element->transform.y -= origin_offset.y;
	return origin_offset;
}

static void CalculateRadius(UIElement *element, UIClass *class){
	for(int i = 0; i < 4; i++){
		switch(class->radius_type.v[i]){
			case UI_PIXELS:
				element->radius.v[i] = class->radius.v[i];
				break;
			default:
				break;
		}
	}
}

static void SetValues(UIElement *element, UIClass *class){
	if(class->color_defined){
		element->color = class->color;
	}
	if(class->border_color_defined){
		element->border_color = class->border_color;
	}
	if(class->text_color_defined){
		element->text_color = class->text_color;
	}
	if(class->text_size_defined){
		element->text_size = class->text_size;
	}
	if(class->font_defined){
		element->font = class->font;
	}
	// if(class->transition_defined){
		// element-> = class->;
	// }
}

// Vector4 CalculateCoverage(UIElement *element){
	
// }

// float Transition(int start, int current, int end, int time_period);

void ApplyClass(UIElement *element, UIClass *class){
	if(class != NULL){
		// --- STYLE APPLICATION ORDER ---
		// Apply defaults (Done once when the element is created)
		// Loop through children to adapt scale (only if scale isnt explicitly set)
		// Loop through classes applying their properties
		CalculateBorder(element, class);
		CalculatePadding(element, class);
		CalculateScale(element, class);
		CalculateMargin(element, class);
		CalculatePosition_relative(element, class);
		CalculateOriginOffset(element, class);
		CalculateRadius(element, class);
		// Calculate transform_relative
		SetValues(element, class);

	}
}

void CalculateChildren(UIElement *element){
	// CalculateElement(element);
	for(int i = 0; i < element->num_classes; i++){
		ApplyClass(element, element->classes[i]);
	}
	CheckInteractions(element);

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
	scene->body.transform = (Vector4){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

}

void tmp(){
	printf("whayo!\n");
}

UIScene *uiLoadFile(UIScene *scene){
	
	scene->num_classes = 0;

	scene->dynamic = false; // Scenes are static by default

	scene->needs_update = true;

	scene->body.name = malloc(sizeof(char) * 5);
	strcpy(scene->body.name, "body");

	scene->body.transform = (Vector4){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	// scene->body.transform = (UI_Property){UI_PIXELS, UI_PIXELS, UI_PIXELS, UI_PIXELS};
	
	scene->body.color = (Vector4){1, 0.5, 0.75, 1};
	scene->body.is_active = true;
	scene->body.parent = NULL;
	scene->body.num_children = 0;


	UIClass *tmp_class = UI_NewClass(scene);

	// tmp_class->transform = (Vector4){20, 20, 0.5, 500};
	// tmp_class->transform_type = (UI_Property){UI_TRANSFORM_PIXELS_INVERTED, UI_PIXELS, UI_PERCENT, UI_PIXELS};
	tmp_class->transform = (Vector4){0.5, 0.5, 60, 60};
	tmp_class->transform_type = (UI_Property){UI_PERCENT, UI_PERCENT, UI_PIXELS, UI_PIXELS};
	
	tmp_class->margin = (Vector4){80, 0, 0, 0};
	tmp_class->margin_type = (UI_Property){UI_PIXELS, UI_PIXELS, UI_PIXELS, UI_PIXELS};
	
	tmp_class->padding = (Vector4){0, 0, 0, 0};
	tmp_class->padding_type = (UI_Property){UI_PIXELS, UI_PIXELS, UI_PIXELS, UI_PIXELS};
	
	// tmp_class->border = (Vector4){2, 0.1, 0, 0};
	// tmp_class->border_type = (UI_Property){UI_PIXELS, UI_PERCENT, UI_PIXELS, UI_PIXELS};
	tmp_class->border = (Vector4){2, 0, 0, 0};
	tmp_class->border_type = (UI_Property){UI_PIXELS, UI_PIXELS, UI_PIXELS, UI_PIXELS};
	
	tmp_class->radius = (Vector4){50, 5, 50, 10};
	tmp_class->radius_type = (UI_Property){UI_PIXELS, UI_PIXELS, UI_PIXELS, UI_PIXELS};

	tmp_class->color = (Vector4){0, 1, 0.5, 1};
	tmp_class->color_defined = true;
	tmp_class->border_color = (Vector4){0, 0, 0.5, 1};
	tmp_class->border_color_defined = true;

	tmp_class->origin = UI_ORIGIN_BOTTOM_RIGHT;
	// tmp_class->origin = UI_ORIGIN_TOP_LEFT;
	// test->origin_set = true;

	// tmp_class->color = (Vector4){0.75, 0.1, 0.5, 1};

	tmp_class->font = &default_font;
	tmp_class->font_defined = true;
	tmp_class->text_size = 1;
	tmp_class->text_size_defined = true;
	tmp_class->text_color = (Vector4){1, 1, 1, 1};
	tmp_class->text_color_defined = true;

	tmp_class->actions[0].num_classes = 0;
	tmp_class->actions[1].num_classes = 0;
	tmp_class->actions[2].num_classes = 0;
	tmp_class->actions[3].num_classes = 0;
	tmp_class->actions[4].num_classes = 0;
	tmp_class->actions[0].function = NULL;
	tmp_class->actions[1].function = NULL;
	tmp_class->actions[2].function = NULL;
	tmp_class->actions[3].function = NULL;
	tmp_class->actions[4].function = NULL;
	tmp_class->actions[UI_ACT_ENTER].function = tmp;


	UIElement *test = UI_NewElement(&scene->body);
	test->children = NULL;
	test->parent = &scene->body;
	test->text = malloc(sizeof(char) * 6);
	test->text_size = 1;
	test->font = &default_font;

	test->color = (Vector4){1, 1, 1, 1};
	test->border_color = (Vector4){0, 0, 0, 1};
	test->text_color = (Vector4){1, 1, 1, 1};

	test->transform = (Vector4){0, 0, 20, 20};

	strcpy(test->text, "test;");
	test->text[5] = 0;

	

	test->num_classes = 0;
	test->classes = malloc(sizeof(UIClass) * (test->num_classes + 1));
	test->classes[test->num_classes++] = tmp_class;
	test->classes = realloc(test->classes, sizeof(UIClass) * (test->num_classes + 1));
	test->is_active = true;

	// test->text = NULL;



	for(int i = 0; i < scene->body.num_children; i++){
		CalculateChildren(&scene->body.children[i]);
	}

	return NULL;
}