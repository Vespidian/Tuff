#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "vectorlib.h"
#include "sdl_gl_init.h"
#include "event.h"
#include "textures.h"
#include "shader.h"
#include "renderer/renderer.h"
#include "render_text.h"

#include "engine.h"
#include "ui.h"

extern Texture texture;

Shader ui_shader;

AttribArray ui_vao;
GLTF ui_gltf;
Mesh ui_mesh;

static void InitUIGL(){
	ui_gltf = GLTFOpen("../assets/models/plane.gltf");
	ui_mesh = ui_gltf.meshes[0];

	if(!ui_mesh.gl_data.is_loaded){
		// set up 'model.renderer' (vao and vbo)
		glGenVertexArrays(1, &ui_mesh.gl_data.vao);
		glBindVertexArray(ui_mesh.gl_data.vao);

		glGenBuffers(1, &ui_mesh.gl_data.pos_vbo);
		glGenBuffers(1, &ui_mesh.gl_data.ebo);

		// Position
		if(ui_mesh.position_exists){
			glBindBuffer(GL_ARRAY_BUFFER, ui_mesh.gl_data.pos_vbo);
			glBufferData(GL_ARRAY_BUFFER, ui_mesh.position_bytelength, ui_mesh.data + ui_mesh.position_offset, GL_STATIC_DRAW);
			// glEnableVertexAttribArray(0);
			glEnableVertexArrayAttrib(ui_mesh.gl_data.vao, 0);
			glVertexAttribPointer(0, ui_mesh.position_size, ui_mesh.position_gl_type, GL_FALSE, 0, (void*)(0));
		}

		if(ui_mesh.index_exists){
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_mesh.gl_data.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, ui_mesh.index_bytelength, ui_mesh.data + ui_mesh.index_offset, GL_STATIC_DRAW);
		}

		ui_mesh.gl_data.is_loaded = true;
	}

	ui_vao = NewVAO(ui_mesh.gl_data.vao, 1, 6, ATTR_VEC3, ATTR_VEC2, ATTR_VEC3, ATTR_VEC3, ATTR_VEC4, ATTR_VEC4);
}

void InitUIRender(){
	InitUIGL();

	ui_shader = ShaderOpen("../assets/shaders/ui.shader");
	ShaderUniformSetSampler2D(&ui_shader, "src_texture", 0);
	ShaderUniformSetMat4(&ui_shader, "tex_coordinates", default_texture_coordinates);

	// Setup global uniforms
	glUniformBlockBinding(ui_shader.id, glGetUniformBlockIndex(ui_shader.id, "ShaderGlobals"), 0);
}



void UIRenderElement(UIElement *element){
	if(element != NULL && element->parent != NULL && element->visible){
		Vector3 r = {
			element->transform.x,
			element->transform.y,
			1
		};
		Vector2 scale = {
			element->transform.z,
			element->transform.w
		};

		float data[64];
		memcpy(&data[0], r.v, sizeof(Vector3));
		memcpy(&data[3], scale.v, sizeof(Vector2));
		memcpy(&data[5], element->style.color.v, sizeof(Vector3));
		memcpy(&data[8], element->style.border_color.v, sizeof(Vector3));
		memcpy(&data[11], element->style.border.v, sizeof(iVector4));

		Texture texture_array[16] = {texture};

		AppendInstance(ui_vao, data, ui_mesh, &ui_shader, 1, texture_array);

		if(element->text != NULL){
			RenderText(
				&default_font, 
				1.001, 
				element->transform.x + element->style.border.x + element->style.padding.x, 
				element->transform.y + element->style.border.y + element->style.padding.y, 
				TEXT_ALIGN_LEFT,
				element->text
			);
		}

	}
}

// void UIRenderSubElements(UIElement *element){
void UIRender(UIState *state){
	if(state != NULL){

		// Root element
		UIElement *element = &state->elements[0];
		
		// Initially we create an array of all children
		UIElement **children;
		unsigned int num_children = 1;
		children = malloc(sizeof(UIElement) * (num_children + 1));
		children[0] = element;
		children[num_children + 1] = NULL;

		if(element->num_children != 0){
			for(int i = 0; children[i] != NULL; i++){
				if((children[i]->num_children == 0) || (children[i]->visible_children == false)){
					continue;
				}

				num_children += children[i]->num_children;
				children = realloc(children, sizeof(UIElement) * (num_children + 1));

				for(int k = 0; k < children[i]->num_children; k++){
					children[num_children - children[i]->num_children + k] = children[i]->children[k];
				}
				children[num_children] = NULL;
			}

			
		}else{
			UIRenderElement(element);
		}

		// TODO: take a look here, does the below loop not 
		// render 'element' twice if num_childre is zero?

		// Loop from leaves to root
		for(int i = (num_children - 1); i >= 0; i--){
			UIRenderElement(children[i]);
		}

		free(children);
		children = NULL;
	}
}