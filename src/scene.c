#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "json_base.h"
#include "vectorlib.h"
#include "material.h"
#include "gltf.h"
#include "bundle.h"
#include "gl_utils.h"

#include "scene.h"

Transform TransformNew(){
	Transform transform;
	transform.position = (Vector3){0, 0, 0};
	transform.scale = (Vector3){1, 1, 1};
	transform.rotation_e = (Vector3){0, 0, 0};
	transform.rotation_q = (Vector4){0, 0, 0, 0};
	glm_mat4_identity(transform.matrix);
	return transform;
}

void TransformCalculate(Transform *transform){
	// translate, rotate, scale
	mat4 translation;
	mat4 rotation;
	mat4 scale;

	glm_mat4_identity(translation);
	glm_mat4_identity(rotation);
	glm_mat4_identity(scale);
	glm_mat4_identity(transform->matrix);

	glm_translate(translation, transform->position.v);
	glm_euler(transform->rotation_e.v, rotation);
	glm_scale(scale, transform->scale.v);

	glm_mul(translation, rotation, transform->matrix);
	glm_mul(transform->matrix, scale, transform->matrix);
}



Renderer RendererNew(){
	Renderer renderer;
	renderer.vao = 0;
	
	renderer.pos_vbo = 0;
	renderer.norm_vbo = 0;
	renderer.uv0_vbo = 0;
	renderer.uv1_vbo = 0;
	renderer.tan_vbo = 0;

	renderer.ebo = 0; // Index buffer
	return renderer;
}

Model ModelNewEmpty(){
	Model model;

	model.parent = NULL;

	model.is_loaded = false;

	model.num_children = 0;
	model.children = NULL;

	model.transform = TransformNew();

	model.renderer = RendererNew();

	model.mesh_path = NULL;
	model.mesh = NULL;

	model.material_path = NULL;
	model.material = NULL;

	return model;
}

Model ModelNew(Model *parent, Mesh *mesh, Material *material){
	Model model = ModelNewEmpty();

	if(mesh != NULL && material != NULL){
		model.parent = parent;

		model.mesh = mesh;
		model.mesh_path = malloc(strlen(mesh->path) + 1);
		memcpy(model.mesh_path, mesh->path, strlen(mesh->path));

		model.material = material;
		model.material_path = malloc(strlen(material->path) + 1);
		memcpy(model.material_path, material->path, strlen(material->path));

		// set up 'model.renderer' (vao and vbo)
		glGenVertexArrays(1, &model.renderer.vao);
		glBindVertexArray(model.renderer.vao);

		glGenBuffers(1, &model.renderer.pos_vbo);
		glGenBuffers(1, &model.renderer.norm_vbo);
		glGenBuffers(1, &model.renderer.uv0_vbo);
		glGenBuffers(1, &model.renderer.uv1_vbo);
		glGenBuffers(1, &model.renderer.tan_vbo);
		glGenBuffers(1, &model.renderer.ebo);


		// Position
		if(model.mesh->position_exists){
			glBindBuffer(GL_ARRAY_BUFFER, model.renderer.pos_vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh->position_bytelength, mesh->data + mesh->position_offset, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, mesh->position_size, mesh->position_gl_type, GL_FALSE, 0, (void*)(0));
		}

		// Normal
		if(model.mesh->normal_exists){
			glBindBuffer(GL_ARRAY_BUFFER, model.renderer.norm_vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh->normal_bytelength, mesh->data + mesh->normal_offset, GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, mesh->normal_size, mesh->normal_gl_type, GL_FALSE, 0, (void*)(0));
		}

		// UV0
		if(model.mesh->uv0_exists){
			glBindBuffer(GL_ARRAY_BUFFER, model.renderer.uv0_vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh->uv0_bytelength, mesh->data + mesh->uv0_offset, GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, mesh->uv0_size, mesh->uv0_gl_type, GL_FALSE, 0, (void*)(0));
		}
		
		// UV1
		if(model.mesh->uv1_exists){
			glBindBuffer(GL_ARRAY_BUFFER, model.renderer.uv1_vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh->uv1_bytelength, mesh->data + mesh->uv1_offset, GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(2, mesh->uv1_size, mesh->uv1_gl_type, GL_FALSE, 0, (void*)(0));
		}
		
		// Tangent
		if(model.mesh->tangent_exists){
			glBindBuffer(GL_ARRAY_BUFFER, model.renderer.tan_vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh->tangent_bytelength, mesh->data + mesh->tangent_offset, GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(2, mesh->tangent_size, mesh->tangent_gl_type, GL_FALSE, 0, (void*)(0));
		}

		// Indices
		if(model.mesh->index_exists){
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.renderer.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_bytelength, mesh->data + mesh->index_offset, GL_STATIC_DRAW);
		}

		model.is_loaded = true;
	}
	return model;
}

void ModelRender(Model *model){
	if(model != NULL && model->is_loaded){
		TransformCalculate(&model->transform);

		UniformSetMat4(model->material->shader, "model", model->transform.matrix); // TODO: Make this uniform buffer objects (UBO)
		MaterialShaderSet(model->material);

		SetVAO(model->renderer.vao);

		if(model->mesh->index_exists){
			glDrawElements(GL_TRIANGLES, model->mesh->index_count, model->mesh->index_gl_type, NULL);
		}else{
			glDrawArrays(GL_TRIANGLES, 0, model->mesh->position_count);
		}
	}
}

void ModelFree(Model *model){
	// Needs to be recursive to free children too

}