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



Model ModelNewEmpty(){
	Model model;

	model.parent = NULL;

	model.is_loaded = false;

	model.num_children = 0;
	model.children = NULL;

	model.transform = TransformNew();

	model.mesh_path = NULL;
	model.mesh = NULL;

	model.material_path = NULL;
	model.material = NULL;

	return model;
}

void MeshPassToGL(Mesh *mesh){
	if(!mesh->gl_data.is_loaded){
		// set up 'model.renderer' (vao and vbo)
		glGenVertexArrays(1, &mesh->gl_data.vao);
		glBindVertexArray(mesh->gl_data.vao);

		glGenBuffers(1, &mesh->gl_data.pos_vbo);
		glGenBuffers(1, &mesh->gl_data.norm_vbo);
		glGenBuffers(1, &mesh->gl_data.uv0_vbo);
		glGenBuffers(1, &mesh->gl_data.uv1_vbo);
		glGenBuffers(1, &mesh->gl_data.tan_vbo);
		glGenBuffers(1, &mesh->gl_data.ebo);

		// Position
		if(mesh->position_exists){
			glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_data.pos_vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh->position_bytelength, mesh->data + mesh->position_offset, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, mesh->position_size, mesh->position_gl_type, GL_FALSE, 0, (void*)(0));
		}

		// Normal
		if(mesh->normal_exists){
			glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_data.norm_vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh->normal_bytelength, mesh->data + mesh->normal_offset, GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, mesh->normal_size, mesh->normal_gl_type, GL_FALSE, 0, (void*)(0));
		}

		// UV0
		if(mesh->uv0_exists){
			glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_data.uv0_vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh->uv0_bytelength, mesh->data + mesh->uv0_offset, GL_STATIC_DRAW);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, mesh->uv0_size, mesh->uv0_gl_type, GL_FALSE, 0, (void*)(0));
		}
		
		// UV1
		if(mesh->uv1_exists){
			glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_data.uv1_vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh->uv1_bytelength, mesh->data + mesh->uv1_offset, GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(2, mesh->uv1_size, mesh->uv1_gl_type, GL_FALSE, 0, (void*)(0));
		}
		
		// Tangent
		if(mesh->tangent_exists){
			glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_data.tan_vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh->tangent_bytelength, mesh->data + mesh->tangent_offset, GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(2, mesh->tangent_size, mesh->tangent_gl_type, GL_FALSE, 0, (void*)(0));
		}

		// Indices
		if(mesh->index_exists){
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->gl_data.ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_bytelength, mesh->data + mesh->index_offset, GL_STATIC_DRAW);
		}

		mesh->gl_data.is_loaded = true;
	}
}

Model ModelNew(Model *parent, Mesh *mesh, Material *material){
	Model model = ModelNewEmpty();

	model.parent = parent;

	if(mesh == NULL){
		model.mesh = &undefined_mesh;
	}else{
		model.mesh = mesh;
		model.mesh_path = malloc(strlen(mesh->path) + 1);
		memcpy(model.mesh_path, mesh->path, strlen(mesh->path));
	}
	MeshPassToGL(mesh);

	if(material == NULL){
		model.material = &undefined_material;
	}else{
		model.material = material;
		model.material_path = malloc(strlen(material->path) + 1);
		memcpy(model.material_path, material->path, strlen(material->path));
	}

	model.is_loaded = true;

	return model;
}

void ModelSetMesh(Model *model, Mesh *mesh){
	if(model != NULL && mesh != NULL){
		// Free the old mesh path
		free(model->mesh_path);
		model->mesh_path = NULL;

		// If the mesh ptr is NULL point it to 'undefined_mesh'
		if(model->mesh == NULL){
			model->mesh = &undefined_mesh;
		}

		// Assign the new mesh data
		model->mesh = mesh;
		model->mesh_index = mesh->mesh_index;

		// Allocate and copy the new mesh path
		model->mesh_path = malloc(strlen(mesh->path) + 1);
		memcpy(model->mesh_path, mesh->path, strlen(mesh->path));
		model->mesh_path[strlen(mesh->path)] = 0;
	}
}

void ModelSetMaterial(Model *model, Material *material){
	if(model != NULL && material != NULL){
		// Free the old material path
		free(model->material_path);
		model->material_path = NULL;

		// If the material ptr is NULL point it to 'undefined_material'
		if(model->material == NULL){
			model->material = &undefined_material;
		}

		// Assign the new material data
		model->material = material;

		// Allocate and copy the new material path
		model->material_path = malloc(strlen(material->path) + 1);
		memcpy(model->material_path, material->path, strlen(material->path));
		model->material_path[strlen(material->path)] = 0;
	}
}

void ModelRender(Model *model){
	if(model != NULL && model->is_loaded){
		TransformCalculate(&model->transform);

		ShaderUniformSetMat4(model->material->shader, "model", model->transform.matrix); // TODO: Make this uniform buffer objects (UBO)
		// Looking back at this: can you even put this uniform into the UBO? SHOULD we even do that?
		MaterialShaderPassUniforms(model->material);

		// SetVAO(model->renderer.vao);
		if(model->mesh != NULL){
			if(!model->mesh->gl_data.is_loaded){
				MeshPassToGL(model->mesh);
			}
		}else{
			model->mesh = &undefined_mesh;
			model->mesh_index = undefined_mesh.mesh_index;
		}
		SetVAO(model->mesh->gl_data.vao);

		for(int i = 0; i < model->material->num_uniforms; i++){
			if(model->material->uniforms[i].type == UNI_SAMPLER2D){
				if(current_texture_unit != i){
					glActiveTexture(GL_TEXTURE0 + i);
					current_texture_unit = i;
				}
				glBindTexture(GL_TEXTURE_2D, model->material->uniforms[i].value._sampler2D);	
				bound_textures[i] = model->material->uniforms[i].value._sampler2D;
			}
		}

		if(model->mesh->index_exists){
			glDrawElements(GL_TRIANGLES, model->mesh->index_count, model->mesh->index_gl_type, NULL);
		}else{
			glDrawArrays(GL_TRIANGLES, 0, model->mesh->position_count);
		}
	}
}

void ModelFree(Model *model){
	// Needs to be recursive to free children too


	model->parent = NULL;
	
	free(model->mesh_path);
	model->mesh_path = NULL;

	model->mesh = NULL;

	free(model->material_path);
	model->material_path = NULL;

	model->material = NULL;

	free(model->children);
	model->children = NULL;
}