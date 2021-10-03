#include "../global.h"
#include "../renderer/renderer.h"
#include "../debug.h"
#include "../gl_context.h"
#include "../event.h"

#include "ui.h"
#include "ui_layout.h"
#include "ui_parser.h"
#include "ui_interact.h"

ShaderObject ui_shader;
AttribArray ui_vao;

UIScene *scene_stack;
unsigned int num_scenes = 0;

static void WindowResize(EventData event){
	if(event.e->window.event == SDL_WINDOWEVENT_RESIZED){
		for(int i = 0; i < num_scenes; i++){
			if(scene_stack[i].body.full_screen){
				scene_stack[i].body.transform.z = SCREEN_WIDTH;
				scene_stack[i].body.transform.w = SCREEN_WIDTH;
			}
		}
	}
}

void InitUI(){
	ui_vao = NewVAO(5, ATTR_MAT4, ATTR_VEC4, ATTR_VEC4, ATTR_VEC4, ATTR_VEC4);
	ui_shader = LoadShaderProgram("ui.shader");
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	glUniformBlockBinding(ui_shader.id, glGetUniformBlockIndex(ui_shader.id, "ShaderGlobals"), 0);
	UniformSetMat4(&ui_shader, "tex_coordinates", default_texture_coordinates);

	BindEvent(EV_POLL_ACCURATE, SDL_WINDOWEVENT, WindowResize);

	DebugLog(D_ACT, "Initialized UI subsystem");
}

void RenderUIElement(UIElement *element, unsigned int layer){
	if(element->text != NULL){
		RenderTextEx(
			&default_font, 
			element->text_size, 
			element->transform.x + element->border.w + element->padding.w,
			element->transform.y + element->border.x + element->padding.x,
			element->text_color,
			TEXT_ALIGN_LEFT,
			100 + layer + 1,
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
	glm_translate(matrix, (vec3){transform.x, transform.y, 100 + layer});
	glm_scale(matrix, (vec2){transform.z, transform.w});

	float data[64] = {0};
	memcpy(&data[0], matrix, sizeof(mat4));
	memcpy(&data[16], element->border.v, sizeof(iVector4));
	memcpy(&data[20], element->radius.v, sizeof(iVector4));
	memcpy(&data[24], element->color.v, sizeof(iVector4));
	memcpy(&data[28], element->border_color.v, sizeof(iVector4));

	TextureObject texture_array[16] = {1};
	AppendInstance(ui_vao, data, &ui_shader, 1, texture_array);
}

static void RecursiveRender(UIElement *element, unsigned int layer){
	if(element->is_active){
		RenderUIElement(element, layer);
		if(element->children != NULL){
			for(int i = 0; i < element->num_children; i++){
				RecursiveRender(&element->children[i], layer + 1);
			}
		}
	}
}

void UI_RenderScene(UIScene *scene){
		for(int i = 0; i < scene->body.num_children; i++){
			// if(scene->needs_update){
			RecursiveApplyStaticClasses(&scene->body.children[i]);
			RecursiveApplyElementClasses(&scene->body.children[i]);
			RecursiveCheckInteract(&scene->body.children[i]);
			RecursiveApplyElementClasses(&scene->body.children[i]);
			// }
		}
		// scene->needs_update = false;


	for(int i = 0; i < scene->body.num_children; i++){
		RecursiveRender(&scene->body.children[i], 0);
	}
}

UIScene *UI_LoadScene(char *path){// TODO: Safeguard the reallocation of the scene_stack so that 'element.scene' can never point to garbage
	scene_stack = realloc(scene_stack, sizeof(UIScene) * (num_scenes + 1));
	
	InitializeScene(&scene_stack[num_scenes]);

	LoadScene(path, &scene_stack[num_scenes]);

	return &scene_stack[num_scenes++];
}

void FreeClass(UIClass *class){
	free(class->name);
	for(int i = 0; i < UI_NUM_ACTIONS; i++){
		free(class->actions[i].classes);
	}
}

void RecursiveFreeElement(UIElement *element){
	for(int i = 0; i < element->num_children; i++){
		RecursiveFreeElement(&element->children[i]);
	}
	free(element->name);
	free(element->children);
	free(element->text);
	free(element->classes);
}

void UI_FreeScene(UIScene *scene){
	RecursiveFreeElement(&scene->body);

	for(int i = 0; i < scene->num_classes; i++){
		FreeClass(&scene->classes[i]);
	}
	free(scene->classes);

	free(scene->path);

	num_scenes--;
	// scene_stack = NULL;
	scene = NULL;
	// TODO: decide whether we want to reload all ui scenes at once or individually
	// Empty all:
	// free the entire scene stack and set it to null. load the scene files
	// Individual frees:
	// set the specified scene to null, shift over all other scenes to fill the newly created gap
}