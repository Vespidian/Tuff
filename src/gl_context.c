#include <math.h>
#include <cglm/cglm.h>
#include "global.h"
#include "shader.h"

#include "event.h"
#include "debug.h"
#include "renderer/renderer.h"
#include "renderer/quad.h"

#include "gl_context.h"
#include "bundle.h"


#include "gltf.h"
#include "scene.h"

SDL_GLContext gl_context;

int z_depth = 1000;

Texture crate_tex;
Texture normal_map;

static void WindowResize(EventData event);
static void KeyPresses(EventData event);
static void Zoom(EventData event);
static void MouseEvent(EventData event);

//testing
mat4 perspective_projection;
mat4 view_matrix;
// unsigned int mesh_vao;
// unsigned int mesh_ebo;

unsigned int vert_vbo;
unsigned int norm_vbo;
unsigned int texc_vbo;
unsigned int tanv_vbo;

// Shader mesh_shader;
mat4 mesh_matrix;

unsigned int uniform_buffer;


float axis[] = {
//  x  y  z   r  g  b   x  y  z   r  g  b 
	1, 0, 0,  1, 0, 0,	0, 0, 0,  1, 0, 0, // x axis
	0, 1, 0,  0, 1, 0,	0, 0, 0,  0, 1, 0, // y axis
	0, 0, 1,  0, 0, 1,	0, 0, 0,  0, 0, 1, // z axis
};
unsigned int axis_vao;
unsigned int axis_vbo;
Shader axis_shader;


float *grid = NULL;
int grid_vertex_count;
int num_grid_verts;
unsigned int grid_vao;
unsigned int grid_vbo;
Shader grid_shader;
bool grid_enabled = true;

void ToggleGrid(){
	grid_enabled = !grid_enabled;
}

void GenerateGrid(){
	Vector3 grid_position = {0, 0, 0};
	
	float cell_size = 0.5;
	int num_cells = 32;
	int divisions_per_cell = 5;

	float grid_size = cell_size * num_cells;
	int num_lines = num_cells * divisions_per_cell + 1;
	float division_spacing = cell_size / divisions_per_cell;

	grid = malloc(grid_vertex_count = (sizeof(float) * 24 * num_lines));
	num_grid_verts = num_lines * 2;
	// memset(grid, grid_position.z, sizeof(float) * 3 * (num_cells * divisions_per_cell + 1));

	for(int i = -num_lines / 2; i < num_lines / 2 + 1; i++){
		// if(i == 0){
		// 	continue;
		// }
		int offset = (i + num_lines / 2) * 24;
		float color = (i % divisions_per_cell == 0) ? 0.55 : 0.4;
		// int offset = (i) * 6;
		// x aligned
		// start
		grid[offset + 0] = -grid_size / 2; 			// x
		grid[offset + 1] = grid_position.y; 		// y
		grid[offset + 2] = i * division_spacing; 	// z
		//color
		grid[offset + 3] = color; // r
		grid[offset + 4] = color; // g
		grid[offset + 5] = color; // b

		// end
		grid[offset + 6] = grid_size / 2; 			// x
		grid[offset + 7] = grid_position.y; 		// y
		grid[offset + 8] = i * division_spacing; 	// z
		//color
		grid[offset + 9 ] = color; // r
		grid[offset + 10] = color; // g
		grid[offset + 11] = color; // b

		// z aligned
		// start
		grid[offset + 12] = i * division_spacing; 	// x
		grid[offset + 13] = grid_position.y; 		// y
		grid[offset + 14] = -grid_size / 2; 			// z
		//color
		grid[offset + 15] = color; // r
		grid[offset + 16] = color; // g
		grid[offset + 17] = color; // b
		
		// end
		grid[offset + 18] = i * division_spacing; 	// x
		grid[offset + 19] = grid_position.y; 		// y
		grid[offset + 20] = grid_size / 2; 			// z
		//color
		grid[offset + 21] = color; // r
		grid[offset + 22] = color; // g
		grid[offset + 23] = color; // b
		
	}
}

char uniform_types[12][16] = {
	"bool",
	"int",
	"float",
	"vec2",
	"vec3",
	"vec4",
	"mat2",
	"mat3",
	"mat4",
	"sampler1d",
	"sampler2d",
	"sampler3d",
};
void PrintShaderUniforms(Shader *shader){
	printf("Shader '%s' uniforms:\n", shader->name);
	for(int i = 0; i < shader->num_uniforms; i++){
		printf("%s : %s\n", shader->uniforms[i].name, uniform_types[shader->uniforms[i].type]);
	}
}

Model model;
Model model2;
Model light_model;

int InitGL(){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
	glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	// glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.258, 0.258, 0.258, 1);
	glm_ortho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, -z_depth / 2, z_depth / 2, orthographic_projection);

	BindEvent(EV_POLL_ACCURATE, SDL_WINDOWEVENT, WindowResize);
    BindEvent(EV_POLL_QUICK, SDL_KEYDOWN, KeyPresses);
    BindEvent(EV_POLL_ACCURATE, SDL_MOUSEWHEEL, Zoom);
    BindEvent(EV_POLL_ACCURATE, SDL_MOUSEMOTION, MouseEvent);
    BindEvent(EV_POLL_ACCURATE, SDL_MOUSEBUTTONDOWN, MouseEvent);
    BindKeyEvent(&ToggleGrid, 'g', SDL_KEYDOWN);

	InitGLUtils();

	RendererInit();

	// FILE LOADING SHOULD NOT HAPPEN IN THIS FUNCTION (should all be in the bundles)
	// crate_tex = TextureOpen("../images/brick_diffuse.png");
	// crate_tex = TextureOpen("images/brick_diffuse.png");
	// normal_map = TextureOpen("images/brick_normal.png");

	axis_shader = ShaderOpen("shaders/axis.shader");
	grid_shader = ShaderOpen("shaders/grid.shader");

	glGenBuffers(1, &uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	// glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * (16 * 2 + 1), NULL, GL_STATIC_DRAW);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * (16 * 3 + 4), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);

	glUniformBlockBinding(axis_shader.id, glGetUniformBlockIndex(axis_shader.id, "ShaderGlobals"), 0);
	glUniformBlockBinding(grid_shader.id, glGetUniformBlockIndex(grid_shader.id, "ShaderGlobals"), 0);
	// glUniformBlockBinding(ui_shader.id, glGetUniformBlockIndex(ui_shader.id, "ShaderGlobals"), 0);

	glGenVertexArrays(1, &axis_vao);
	glBindVertexArray(axis_vao);
	glGenBuffers(1, &axis_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis), axis, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));



	GenerateGrid();
	glGenVertexArrays(1, &grid_vao);
	glBindVertexArray(grid_vao);

	glGenBuffers(1, &grid_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);
	glBufferData(GL_ARRAY_BUFFER, grid_vertex_count, grid, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	
	// glGenVertexArrays(1, &mesh_vao);
	// glBindVertexArray(mesh_vao);

	glGenBuffers(1, &vert_vbo);
	glGenBuffers(1, &norm_vbo);
	glGenBuffers(1, &texc_vbo);
	glGenBuffers(1, &tanv_vbo);

	// glGenBuffers(1, &mesh_ebo);


	// model = ModelNew(NULL, &BundleGLTFFind(&app, "models/soda_can.gltf")->meshes[0], BundleMaterialFind(&app, "materials/default.mat"));
	// Open these once to stabilize the address of the material array
	// BundleShaderFind(&app, "shaders/default.shader");
	// BundleShaderFind(&app, "shaders/squish.shader");
	// BundleShaderFind(&app, "shaders/light.shader");
	// BundleMaterialFind(&app, "materials/default.mat");
	// BundleMaterialFind(&app, "materials/squish.mat");
	// BundleMaterialFind(&app, "materials/light.mat");

	model = ModelNew(NULL, &BundleGLTFFind(&app, "models/entrance.gltf", true)->meshes[0], BundleMaterialFind(&app, "materials/default.mat", true));
	model2 = ModelNew(NULL, &BundleGLTFFind(&app, "models/squish_cube.gltf", true)->meshes[0], BundleMaterialFind(&app, "materials/squish.mat", true));
	light_model = ModelNew(NULL, &BundleGLTFFind(&app, "models/uv_sphere.gltf", true)->meshes[0], BundleMaterialFind(&app, "materials/light.mat", true));

    glm_mat4_identity(perspective_projection);
    glm_mat4_identity(view_matrix);

    // set projection transform
    glm_perspective(glm_rad(90), SCREEN_WIDTH / SCREEN_HEIGHT, 0.01, 100, perspective_projection);

	// ShaderUniformSetSampler2D(BundleShaderFind(&app, "shaders/default.shader"), "tex", 0);
	// ShaderUniformSetSampler2D(BundleShaderFind(&app, "shaders/default.shader"), "normal_map", 1);


	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, &perspective_projection);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16, sizeof(float) * 16, &orthographic_projection);

    DebugLog(D_ACT, "Initialized OpenGL");

	// mesh_transform.scale = (Vector3){0.5, 0.5, 0.5};
	// InitTransform(&origin_transform);

	// InitScene(&active_scene);
	// parent = NewModel("parent", NULL, NULL, 0, 0, 0);
	// child = NewModel("child", parent, NULL, 0, 0, 0);


	// MaterialShaderPassUniforms(BundleMaterialFind(&app, "materials/default.mat"));

    GLCall;
	// PrintShaderUniforms(&mesh_shader);

    return 0;
}

void ExitGL(){

	ShaderFree(&axis_shader);
	ShaderFree(&grid_shader);

}

#include "renderer/render_text.h"

// static float value = 0;
float light_time = 0;
// Vector3 light_color = {0, 1, 0};
Vector3 light_color = {0.98, 0.839, 0.647};
// TransformObject light_transform = {{1, 0.6, 1.2}, {1, 1, 1}, {0, 0, 0}};
static Vector3 direction = {0, 0, 0};
static Vector3 view_position = {0, 0, 0};
// static Vector3 mesh_position = {0, 0.01, 0};
Vector3 light_position = {1, 2, 1.2};
static float view_distance = 5;
static float yaw, pitch = 0.25;
static float normal_intense = 1;

void RenderGL(){
	// RenderUI();

	Vector3 tmp = {0, 0, view_distance};
	glm_vec3_rotate(tmp.v, -pitch * 2, (vec3){1, 0, 0});
	glm_vec3_rotate(tmp.v, -yaw, (vec3){0, 1, 0});
	// float w = view_distance * cos(pitch);
	glm_vec3_add(view_position.v, tmp.v, tmp.v);
	// ShaderUniformSetVec3(mesh_shader, "view_position", tmp.v);
	ShaderUniformSetVec3(BundleShaderFind(&app, "shaders/default.shader", true), "view_position", tmp.v);


	ShaderUniformSetFloat(BundleShaderFind(&app, "shaders/default.shader", true), "normal_map_intensity", normal_intense);

    glm_mat4_identity(view_matrix);
	glm_translate(view_matrix, (vec3){0, 0, -view_distance});
	glm_rotate(view_matrix, pitch * 2, (vec3){1, 0, 0});
	glm_rotate(view_matrix, yaw, (vec3){0, 1, 0});
	glm_translate(view_matrix, view_position.v);
	
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16 * 2, sizeof(float) * 16, &view_matrix);

	// Set the time uniform
	float time = SDL_GetTicks() / 1000.0;
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16 * 3, sizeof(float) * 4, &time);


	// RETURN: Something weird is happening when reloading the bundle / shaders
	// the normal map texture slot is being set to the regular brick texture
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, BundleTextureFind(&app, "images/brick_diffuse.png", true)->gl_tex);
	current_texture_unit = 0;
	bound_textures[0] = BundleTextureFind(&app, "images/brick_diffuse.png", true)->gl_tex;

	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, BundleTextureFind(&app, "images/brick_diffuse.png", true)->gl_tex);
	current_texture_unit = 1;
	bound_textures[1] = BundleTextureFind(&app, "images/brick_diffuse.png", true)->gl_tex;


	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	mat4 model_matrix;
	glm_mat4_identity(model_matrix);
	// Scene axis
	{
		SetVAO(axis_vao);
		ShaderUniformSetFloat(&axis_shader, "zoom", view_distance);
		ShaderUniformSetMat4(&axis_shader, "model", model_matrix);
		ShaderPassUniforms(&axis_shader);
		glDrawArrays(GL_LINES, 0, 26);
	}


	if(grid_enabled){
		SetVAO(grid_vao);
		// glBindVertexArray(grid_vbo);
		ShaderPassUniforms(&grid_shader);
		glBindVertexBuffer(0, grid_vbo, 0, sizeof(float) * 6);
		glBindVertexBuffer(1, grid_vbo, sizeof(float) * 3, sizeof(float) * 6);
		glDrawArrays(GL_LINES, 0, grid_vertex_count);
	}

	glDisable(GL_MULTISAMPLE);
	glDisable(GL_LINE_SMOOTH);

	light_position.x = 2 * sin(time);
	light_position.z = 2 * cos(time);
	light_position.y = -cos(time) / 2 + 1;

	// SetVAO(mesh_vao);
	// ShaderPassUniforms(&mesh_shader);
	// ShaderUniformSetVec3(BundleShaderFind(&app, "shaders/default.shader"), "")
    // ShaderUniformSetMat4(BundleShaderFind(&app, "shaders/default.shader"), "model", model_matrix);
    // ShaderUniformSetMat4(BundleShaderFind(&app, "shaders/default.shader"), "model", model_matrix);
	// ShaderPassUniforms(BundleShaderFind(&app, "shaders/default.shader"));
	// glDrawElements(GL_TRIANGLES, app.meshes->accessors[app.meshes->meshes[0].indices].count, app.meshes->accessors[app.meshes->meshes[0].indices].component_type, NULL);
	// glDrawElements(GL_TRIANGLES, app.gltfs[0].meshes[0].index_count, app.gltfs[0].meshes[0].index_gl_type, NULL);
	
	MaterialUniformSetVec3(model.material, "light_color", light_color.v);
	MaterialUniformSetVec3(model.material, "light_pos", light_position.v);
	// MaterialUniformSetSampler2D(model2.material, "normal_map", BundleTextureFind(&app, "images/brick_diffuse.png")->gl_tex);
	model.transform.position = (Vector3){0, 1, 0};
	ModelRender(&model);


	light_model.transform.position = light_position;
	light_model.transform.scale = (Vector3){0.2, 0.2, 0.2};
	MaterialUniformSetVec3(light_model.material, "light_color", light_color.v);	
	ModelRender(&light_model);


	MaterialUniformSetVec3(model2.material, "light_color", light_color.v);
	MaterialUniformSetVec3(model2.material, "light_pos", light_position.v);
	model2.transform.scale.x = model2.transform.scale.y = model2.transform.scale.z = 0.5;
	model2.transform.position.x = -2;
	model2.transform.position.y = sin(time * 2) / 2 + 1;
	ModelRender(&model2);
	// SDL_Rect mouse = {mouse_pos.x, mouse_pos.y, 16, 16};
}

float movement_speed = 0.01f;
float rotation_speed = 0.3f;
float zoom_speed = 0.075f;
static void Zoom(EventData event){
	if(event.e->wheel.y > 0){
		view_distance -= zoom_speed * view_distance;
	}else if(event.e->wheel.y < 0){
		view_distance += zoom_speed * view_distance;
	}
}

static void KeyPresses(EventData event){
	Vector3 direction = {0, 0, 0};
	// Forward / back
	if(event.keyStates[SDL_SCANCODE_W]){
		glm_vec3_add(direction.v, (vec3){0, 0, 1}, direction.v);
		// direction = (Vector3){0, 0, 1};
	}
	if(event.keyStates[SDL_SCANCODE_A]){
		glm_vec3_add(direction.v, (vec3){-1, 0, 0}, direction.v);
	}
	// Left / right
	if(event.keyStates[SDL_SCANCODE_S]){
		glm_vec3_add(direction.v, (vec3){0, 0, -1}, direction.v);
	}
	if(event.keyStates[SDL_SCANCODE_D]){
		glm_vec3_add(direction.v, (vec3){1, 0, 0}, direction.v);
	}
	// Up / Down
	if(event.keyStates[SDL_SCANCODE_LCTRL]){
		glm_vec3_add(direction.v, (vec3){0, 1, 0}, direction.v);
	}
	if(event.keyStates[SDL_SCANCODE_SPACE]){
		glm_vec3_add(direction.v, (vec3){0, -1, 0}, direction.v);
	}

	glm_vec3_scale(direction.v, movement_speed * view_distance, direction.v);
	if(event.keyStates[SDL_SCANCODE_LSHIFT]){
		glm_vec3_scale(direction.v, 3, direction.v);
	}
	glm_vec3_rotate(direction.v, -pitch * 2, (vec3){1, 0, 0});
	glm_vec3_rotate(direction.v, yaw, (vec3){0, 1, 0});
	view_position.x += -direction.x;
	view_position.y += direction.y;
	view_position.z += direction.z;
}

static void WindowResize(EventData event){
	if(event.e->window.event == SDL_WINDOWEVENT_RESIZED){
		SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glm_ortho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, -z_depth / 2, z_depth / 2, orthographic_projection);
		glm_perspective(glm_rad(90), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01, 100, perspective_projection);
		glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, &perspective_projection);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16, sizeof(float) * 16, &orthographic_projection);
	}
}

Vector2 last_pos;
Vector2 current_pos;
static float sensitivity = 0.005f;
static float move_sensitivity = 0.0025f;
static void MouseEvent(EventData event){
	current_pos = (Vector2){mouse_pos.x, mouse_pos.y};
	if(mouse_clicked){
		last_pos = (Vector2)current_pos;
	}

	// if(*event.mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE)){
		if(mouse_held){

			SDL_SetWindowGrab(window, SDL_TRUE);
			Vector2 difference;
			glm_vec2_sub(current_pos.v, last_pos.v, difference.v);

			// parent->transform.rotation_e = (Vector3){-pitch, -yaw, 0};

			if(event.keyStates[SDL_SCANCODE_LSHIFT]){
				Vector3 tmp = {-difference.x * move_sensitivity * view_distance, -difference.y * move_sensitivity * view_distance, 0};
				glm_vec3_rotate(tmp.v, -pitch * 2, (vec3){1, 0, 0});
				glm_vec3_rotate(tmp.v, yaw, (vec3){0, 1, 0});
				view_position.x += -tmp.x;
				view_position.y += tmp.y;
				view_position.z += tmp.z;

			}else{

				yaw += difference.x * sensitivity;
				pitch += difference.y * sensitivity;

				if(pitch > M_PI * 2){
					pitch -= M_PI * 2;
				}
				if(pitch < 0){
					pitch += M_PI * 2;
				}

				if(yaw > M_PI * 2){
					yaw -= M_PI * 2;
				}
				if(yaw < 0){
					yaw += M_PI * 2;
				}

				direction.x = yaw;
				direction.y = pitch;
			}

			if(mouse_pos.x <= 10){
				// SDL_WarpMouseInWindow(window, SCREEN_WIDTH - 11, current_pos.y);
				SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
				current_pos = (Vector2){mouse_pos.x, mouse_pos.y};
			}
			if(mouse_pos.x >= SCREEN_WIDTH - 10){
				// SDL_WarpMouseInWindow(window, 11, current_pos.y);
				SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
				current_pos = (Vector2){mouse_pos.x, mouse_pos.y};
			}
			if(mouse_pos.y <= 10){
				// SDL_WarpMouseInWindow(window, current_pos.x, SCREEN_HEIGHT - 11);
				SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
				current_pos = (Vector2){mouse_pos.x, mouse_pos.y};
			}
			if(mouse_pos.y >= SCREEN_HEIGHT - 10){
				// SDL_WarpMouseInWindow(window, current_pos.x, 11);
				SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
				current_pos = (Vector2){mouse_pos.x, mouse_pos.y};
			}
			last_pos = (Vector2)current_pos;
		}else{
			SDL_SetWindowGrab(window, SDL_FALSE);
		}
	// }
}