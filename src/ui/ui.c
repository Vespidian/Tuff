#include "../global.h"
#include "../renderer/renderer.h"
#include "../debug.h"
#include "../gl_context.h"
#include "../event.h"

#include "ui.h"
#include "ui_layout.h"
#include "ui_parser.h"

TilesheetObject ui_tilesheet;

ShaderObject ui_shader;
AttribArray ui_vao;

UIScene *scene_stack;
unsigned int num_scenes = 0;

static void WindowResized(){
	
}

void InitUI(){
	ui_tilesheet = LoadTilesheet("../images/ui/ui.png", 16, 16);


	ui_vao = NewVAO(5, ATTR_MAT4, ATTR_VEC4, ATTR_VEC4, ATTR_VEC4, ATTR_VEC4);
	ui_shader = LoadShaderProgram("ui.shader");
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	glUniformBlockBinding(ui_shader.id, glGetUniformBlockIndex(ui_shader.id, "ShaderGlobals"), 0);
	UniformSetMat4(&ui_shader, "tex_coordinates", default_texture_coordinates);

	BindEvent(EV_POLL_ACCURATE, SDL_WINDOWEVENT_RESIZED, WindowResized);

	DebugLog(D_ACT, "Initialized UI subsystem");
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

	float data[64];
	memcpy(&data[0], matrix, sizeof(mat4));
	memcpy(&data[16], element->border.v, sizeof(vec4));
	memcpy(&data[20], element->radius.v, sizeof(vec4));
	memcpy(&data[24], element->color.v, sizeof(vec4));
	memcpy(&data[28], element->border_color.v, sizeof(vec4));

	TextureObject texture_array[16] = {1};
	AppendInstance(ui_vao, data, &ui_shader, 1, texture_array);
}

static void RecursiveRender(UIElement *element){
	if(element->is_active){
		RenderUIElement(element);
		if(element->children != NULL){
			for(int i = 0; i < element->num_children; i++){
				RecursiveRender(&element->children[i]);
			}
		}
	}
}

void UI_RenderScene(UIScene *scene){
	// if(scene->needs_update){
	// 	CalculateChildren(&scene->body);
	// }
	if(scene->needs_update){
		for(int i = 0; i < scene->body.num_children; i++){
			RecursiveApplyElementClasses(&scene->body.children[i]);
		}
	}

	for(int i = 0; i < scene->body.num_children; i++){
		RecursiveRender(&scene->body.children[i]);
	}
}

UIScene *NewScene(char *path){
	scene_stack = realloc(scene_stack, sizeof(UIScene) * (num_scenes + 1));

	LoadScene(path, &scene_stack[num_scenes]);

	return &scene_stack[num_scenes++];
}