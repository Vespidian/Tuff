#include <math.h>
#include "global.h"
#include "gl_utils.h"

#include "event.h"
#include "debug.h"
#include "renderer/renderer.h"
#include "renderer/quad.h"
#include "ui/ui.h"

#include "gl_context.h"
#include "scene/scene.h"


// #include "scene/obj_loader.h"
#include "scene/gltf_loader.h"

SDL_GLContext gl_context;

int z_depth = 1000;

TilesheetObject tilesheet_texture;
TextureObject crate_tex;
TextureObject normal_map;

static void WindowResize(EventData event);
static void KeyPresses(EventData event);
static void Zoom(EventData event);
static void MouseEvent(EventData event);

//testing
mat4 perspective_projection;
mat4 view_matrix;
unsigned int uniform_buffer;
unsigned int mesh_vao;
unsigned int mesh_ebo;
unsigned int vert_vbo;
unsigned int norm_vbo;
unsigned int texc_vbo;
ShaderObject mesh_shader;
mat4 mesh_matrix;

float axis[] = {
	// x1, y1, z1, r, g, b, x2, y2, z2, r, g, b,
	0, 0, 0,  1, 0, 0,    1, 0, 0,  1, 0, 0, // x axis
	0, 0, 0,  0, 1, 0,    0, 1, 0,  0, 1, 0, // y axis
	0, 0, 0,  0, 0, 1,    0, 0, 1,  0, 0, 1, // z axis
	// -10, 0, 0,  1, 0, 0,    10, 0, 0,  1, 0, 0, // x axis
	// 0, -10, 0,  0, 1, 0,    0, 10, 0,  0, 1, 0, // y axis
	// 0, 0, -10,  0, 0, 1,    0, 0, 10,  0, 0, 1, // z axis
};
unsigned int axis_vao;
unsigned int axis_vbo;
ShaderObject axis_shader;

// float grid[] = { // floor grid
// 	-1, 0, 1,
// 	1, 0, 1,
// 	1, 0, -1,

// 	-1, 0, 1,
// 	1, 0, -1,
// 	-1, 0, -1,
// };
// float grid[] = { // screen space grid
// 	// vertical quad
// 	// -1, 1, 0,
// 	// 1, 1, 0,
// 	// 1, -1, 0,

// 	// -1, 1, 0,
// 	// 1, -1, 0,
// 	// -1, -1, 0,

// 	// horizontal
// 	-1, 0, 1,
// 	1, 0, 1,
// 	1, 0, -1,

// 	-1, 0, 1,
// 	1, 0, -1,
// 	-1, 0, -1,

// };
float *grid;
int grid_vertex_count;
int num_grid_verts;
unsigned int grid_vao;
unsigned int grid_vbo;
ShaderObject grid_shader;
// TextureObject grid_texture;

void GenerateGrid(){
	// glLineWidth(2);
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
	printf("HERE\n");
}

int InitGL(){
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        DebugLog(D_ERR, "Failed to initialize GLEW!");
        return EXIT_FAILURE;
    }
	SDL_GL_SetSwapInterval(1);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
	glEnable(GL_PROGRAM_POINT_SIZE);
    // glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	// glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glClearColor(0.5f, 0.4f, 0.3f, 1);
	glClearColor(0.258, 0.258, 0.258, 1);
	glm_ortho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, -z_depth / 2, z_depth / 2, orthographic_projection);

	BindEvent(EV_ACCURATE, SDL_WINDOWEVENT, WindowResize);
    BindEvent(EV_QUICK, SDL_KEYDOWN, KeyPresses);
    BindEvent(EV_ACCURATE, SDL_MOUSEWHEEL, Zoom);
    BindEvent(EV_ACCURATE, SDL_MOUSEMOTION, MouseEvent);
    BindEvent(EV_ACCURATE, SDL_MOUSEBUTTONDOWN, MouseEvent);

	printf("SHADER UNIFORM TYPES:\nbool: %d\nfloat: %d\nint: %d\nvec2: %d\nvec3: %d\nvec4: %d\nmat2: %d\nmat3: %d\nmat4: %d\n", UNI_BOOL, UNI_FLOAT, UNI_INT, UNI_VEC2, UNI_VEC3, UNI_VEC4, UNI_MAT2, UNI_MAT3, UNI_MAT4);

	InitGLUtils();

	RendererInit();


    tilesheet_texture = LoadTilesheet("../images/testingTemp/tmpTilesheet.png", GL_RGBA, 16, 16);
	crate_tex = LoadTexture("../images/wood2.png", GL_RGB);
	normal_map = LoadTexture("../images/brick_normal.png", GL_RGBA);
	// grid_texture = LoadTexture("../images/grid.png", GL_RGBA);




	// mesh_shader = LoadShaderProgram("../shaders/mesh.vert", "../shaders/lit.frag");
	// axis_shader = LoadShaderProgram("../shaders/axis.vert", "../shaders/axis.frag");
	// grid_shader = LoadShaderProgram("../shaders/grid.vert", "../shaders/grid.frag");
	mesh_shader = LoadShaderProgram("../shaders/mesh.shader");
	printf("GOT HERE\n");
	axis_shader = LoadShaderProgram("../shaders/axis.shader");
	grid_shader = LoadShaderProgram("../shaders/grid.shader");


	glGenBuffers(1, &uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * (16 * 2 + 1), NULL, GL_STATIC_DRAW);
	// glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);

	glUniformBlockBinding(mesh_shader.id, glGetUniformBlockIndex(mesh_shader.id, "Matrices"), 0);
	glUniformBlockBinding(axis_shader.id, glGetUniformBlockIndex(axis_shader.id, "Matrices"), 0);
	glUniformBlockBinding(grid_shader.id, glGetUniformBlockIndex(grid_shader.id, "Matrices"), 0);


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

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));

// ShaderObject shad = ParseShaderUniforms("test", 0, "uniform vec2 finding_nemo_is_a_sad_story;", "");
// 	printf("nam: %s\n", shad.name);
// 	printf("uni[0] name: %s\n", shad.uniforms[0].name);


	// int start = SDL_GetTicks();
	// LoadObj("../models/cube.obj", &mesh);
	LoadGLTF();
	// printf("Loaded mesh in %dms\n", SDL_GetTicks() - start);
	glGenVertexArrays(1, &mesh_vao);
	glBindVertexArray(mesh_vao);

	glGenBuffers(1, &vert_vbo);
	glGenBuffers(1, &norm_vbo);
	glGenBuffers(1, &texc_vbo);
	glGenBuffers(1, &mesh_ebo);


	// glBufferData(GL_ELEMENT_ARRAY_BUFFER, 72, data->meshes->primitives->indices->buffer_view->buffer->data, GL_STATIC_DRAW);
printf("%d\n", pos->size);

	glBindBuffer(GL_ARRAY_BUFFER, vert_vbo);
	// glBufferData(GL_ARRAY_BUFFER, pos->size + norm->size, pos->buffer->data, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, pos->size + norm->size, pos->buffer->data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));

	glBindBuffer(GL_ARRAY_BUFFER, norm_vbo);
	glBufferData(GL_ARRAY_BUFFER, norm->size, norm->buffer->data + norm->offset, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));

	glBindBuffer(GL_ARRAY_BUFFER, texc_vbo);
	glBufferData(GL_ARRAY_BUFFER, texc->size, texc->buffer->data + texc->offset, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(0));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind->size, data->buffers->data + ind->offset, GL_STATIC_DRAW);

    glm_mat4_identity(perspective_projection);
    glm_mat4_identity(view_matrix);

    // set projection transform
    glm_perspective(glm_rad(90), SCREEN_WIDTH / SCREEN_HEIGHT, 0.01, 100, perspective_projection);

	UniformSetInt(&mesh_shader, "tex", 0);
	UniformSetInt(&mesh_shader, "normal_map", 1);
	// UniformSetVec3_m(&mesh_shader, "color", 1, 0.5, 1);
	UniformSetInt(&grid_shader, "texture_0", 0);


	glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, &perspective_projection);

    DebugLog(D_ACT, "Initialized OpenGL");
	
    GLCall;
    return 0;
}


// static float value = 0;
float light_time = 0;
Vector3 light_color = {1, 1, 1};
TransformObject light_transform = {{1, 0.6, 1.2}, {1, 1, 1}, {0, 0, 0}};
static Vector3 direction = {0, 0, 0};
static Vector3 view_position = {0, 0, 0};
static Vector3 mesh_position = {0, 0.01, 0};
// static Vector3 light_position = {1, 0.6, 1.2};
static float view_distance = 2;
static float yaw, pitch;
#include "ui/elements/slider.h"
#include "ui/ui.h"
#include "renderer/render_text.h"
void RenderUI(){
	Vector3 tmp = {0, 0, view_distance};
	glm_vec3_rotate(tmp.v, -pitch * 2, (vec3){1, 0, 0});
	glm_vec3_rotate(tmp.v, -yaw, (vec3){0, 1, 0});
	// float w = view_distance * cos(pitch);
	glm_vec3_add(view_position.v, tmp.v, tmp.v);
	// UniformSetVec3(mesh_shader, "view_position", tmp.v);
	UniformSetVec3(&mesh_shader, "view_position", tmp.v);

	static float normal_intense;
	Vector2 size = {200, 30};
	Vector2 origin = {SCREEN_WIDTH - size.x / 2 - 10, size.y * 3};
	Vector2 origin2 = {SCREEN_WIDTH - size.x / 2 - 10, size.y * 3};
	RenderSlider(&light_time, 0.0000, 1.0000, VerticalRectList_vec(3, 0, size, origin, 6));
	RenderSlider(&normal_intense, 0.0, 10, VerticalRectList_vec(3, 1, size, origin, 6));
	RenderSlider(&tmp.z, 0.0, 10, VerticalRectList_vec(3, 2, size, origin, 6));
	RenderTextEx(&default_font, 1, VerticalRectList_vec(3, 0, size, origin2, 6).x, VerticalRectList_vec(3, 0, size, origin2, 6).y, (Vector4){1, 0, 0, 1}, TEXT_ALIGN_RIGHT, -1, "light_time: ");
	RenderTextEx(&default_font, 1, VerticalRectList_vec(3, 1, size, origin2, 6).x, VerticalRectList_vec(3, 1, size, origin2, 6).y, (Vector4){0, 1, 0, 1}, TEXT_ALIGN_RIGHT, -1, "y: ");
	RenderTextEx(&default_font, 1, VerticalRectList_vec(3, 2, size, origin2, 6).x, VerticalRectList_vec(3, 2, size, origin2, 6).y, (Vector4){0, 0, 1, 1}, TEXT_ALIGN_RIGHT, -1, "z: ");

	UniformSetFloat(&mesh_shader, "normal_map_intensity", normal_intense);

    glm_mat4_identity(view_matrix);
	glm_mat4_identity(mesh_matrix);

	
	glm_translate(view_matrix, (vec3){0, 0, -view_distance});
	glm_rotate(view_matrix, pitch * 2, (vec3){1, 0, 0});
	glm_rotate(view_matrix, yaw, (vec3){0, 1, 0});
	glm_translate(view_matrix, view_position.v);
	

	// UniformSetMat4(mesh_shader, "view", view_matrix);
	// UniformSetMat4(unlit_shader, "view", view_matrix);
	// UniformSetMat4(grid_shader, "view", view_matrix);
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16, sizeof(float) * 16, &view_matrix);
	RenderText(&default_font, 1, 0, 0, TEXT_ALIGN_LEFT, "view_distance: %f", view_distance);

	// Set the time uniform
	int time = SDL_GetTicks();
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16 * 2, sizeof(float), &time);

    glm_translate(mesh_matrix, mesh_position.v);
	glm_scale_uni(mesh_matrix, 0.5f);
    UniformSetMat4(&mesh_shader, "model", mesh_matrix);

	// float time = SDL_GetTicks() / 1000.0;
	// glm_scale_uni(mesh_matrix, 0.1);
	glm_mat4_identity(light_transform.result);
	glm_translate(light_transform.result, light_transform.position.v);
	glm_vec3_rotate(light_transform.position.v, light_time, (vec3){0, 1, 0});
	light_transform.position.y = cos(light_time * 7.5) / 1.5;
	glm_scale_uni(light_transform.result, 0.25);
	UniformSetVec3(&mesh_shader, "light_pos", light_transform.position.v);

	// Vector3 color = {sin(1), cos(1), 1.2};
	Vector3 color = {0.5, 0.5, 0.5};
	UniformSetVec3(&mesh_shader, "light_color", color.v);


	// glm_vec3_rotate(light_transform.position.v, -light_time, (vec3){0, 1, 0});

	// Vector3 tmp = view_position;
	// Vector3 tmp = {0, 0, -view_distance};
	// glm_vec3_rotate(tmp.v, pitch * 2, (vec3){1, 0, 0});
	// glm_vec3_rotate(tmp.v, yaw, (vec3){0, 1, 0});
	// // float w = view_distance * cos(pitch);
	// glm_vec3_add(view_position.v, tmp.v, tmp.v);
	// // UniformSetVec3(mesh_shader, "view_position", tmp.v);
	// UniformSetVec3(mesh_shader, "view_position", tmp.v);

    // // GLCall;
}

void RenderGL(){
	RenderUI();
	// SetShaderProgram(mesh_shader);

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, crate_tex.gl_tex);
	current_texture_unit = 0;
	bound_textures[0] = crate_tex.gl_tex;

	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normal_map.gl_tex);
	current_texture_unit = 1;
	bound_textures[1] = normal_map.gl_tex;


	// glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_ebo);
	SetVAO(mesh_vao);
	// SetShaderProgram(&mesh_shader);
	PassShaderUniforms(&mesh_shader);

	// glDrawArrays(GL_TRIANGLES, 24, mesh_vbo);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_ebo);
	// glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
	// glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
	int type;
	if(data->meshes->primitives->indices->component_type == cgltf_component_type_r_16u){
		type = GL_UNSIGNED_SHORT;
	}else{
		type = GL_UNSIGNED_INT;
	}
	glDrawElements(GL_TRIANGLES, data->meshes->primitives->indices->count, type, NULL);


	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	SetVAO(axis_vao);
	// SetShaderProgram(&axis_shader);
	PassShaderUniforms(&axis_shader);
	glDrawArrays(GL_LINES, 0, 26);

	// glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, grid_texture.gl_tex);
	// current_texture_unit = 0;
	// bound_textures[0] = grid_texture.gl_tex;
	SetVAO(grid_vao);
	// SetShaderProgram(&grid_shader);
	PassShaderUniforms(&grid_shader);
	// glDrawArrays(GL_TRIANGLES, 0, 6);
	glDrawArrays(GL_LINES, 0, grid_vertex_count);
	glDisable(GL_MULTISAMPLE);
	glDisable(GL_LINE_SMOOTH);
	// glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 192, mesh.data, GL_STATIC_DRAW);
	// glDrawArrays(GL_TRIANGLES, 0, 192);

	// glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.data_size, mesh.data, GL_STATIC_DRAW);

	// glBufferData(GL_ARRAY_BUFFER, sizeof(float) * axis_mesh.data_size, axis_mesh.data, GL_STATIC_DRAW);
	// glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);
	// glBindVertexArray(mesh_vao);
	// SetShaderProgram(grid_shader);
	// glBindVertexArray(axis_vao);
	// current_vao = axis_vao;
	// glDrawArrays(GL_TRIANGLES, 0, mesh.num_vertices);

		// SetShaderProgram(unlit_shader);
	// glDrawArrays(GL_TRIANGLES, 0, mesh.num_vertices);
	// glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, 0);



}

float movement_speed = 0.05f;
float rotation_speed = 0.3f;
float zoom_speed = 0.05f;
static void Zoom(EventData event){
	if(event.e->wheel.y > 0){
		view_distance -= zoom_speed;
	}else if(event.e->wheel.y < 0){
		view_distance += zoom_speed;
	}
}

static void KeyPresses(EventData event){
	// Forward / back
	if(event.keyStates[SDL_SCANCODE_W]){view_position.z += movement_speed;}
	if(event.keyStates[SDL_SCANCODE_A]){view_position.x += movement_speed;}
	// Left / right
	if(event.keyStates[SDL_SCANCODE_S]){view_position.z -= movement_speed;}
	if(event.keyStates[SDL_SCANCODE_D]){view_position.x -= movement_speed;}
	// Up / down
	if(event.keyStates[SDL_SCANCODE_LCTRL]){view_position.y += movement_speed;}
	if(event.keyStates[SDL_SCANCODE_SPACE]){view_position.y -= movement_speed;}

	if(event.keyStates[SDL_SCANCODE_LEFT]){direction.y += rotation_speed;}
	if(event.keyStates[SDL_SCANCODE_RIGHT]){direction.y -= rotation_speed;}
	if(event.keyStates[SDL_SCANCODE_UP]){direction.x += rotation_speed;}
	if(event.keyStates[SDL_SCANCODE_DOWN]){direction.x -= rotation_speed;}
}

static void WindowResize(EventData event){
	if(event.e->window.event == SDL_WINDOWEVENT_RESIZED){
		SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glm_ortho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, -z_depth / 2, z_depth / 2, orthographic_projection);
    	// glm_mat4_identity(perspective_projection);
		// printf("width: %d\nheight: %d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
		glm_perspective(glm_rad(90), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01, 100, perspective_projection);
		glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, &perspective_projection);
		// UniformSetMat4(unlit_shader, "projection", perspective_projection);
		// UniformSetMat4(mesh_shader, "projection", perspective_projection);
		// UniformSetMat4(grid_shader, "projection", perspective_projection);
	}
}

Vector2 last_pos;
Vector2 current_pos;
static float sensitivity = 0.005f;
static float move_sensitivity = 0.01f;
static void MouseEvent(EventData event){
	current_pos = (Vector2){mouse_pos.y, mouse_pos.x};
	if(mouse_clicked && !ui_hovered){
		last_pos = (Vector2)current_pos;
	}
	// if(*event.mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE)){
		if(mouse_held && !ui_hovered){
			Vector2 difference;
			glm_vec2_sub((vec2){mouse_pos.y, mouse_pos.x}, last_pos.v, difference.v);

			if(event.keyStates[SDL_SCANCODE_LSHIFT]){
				vec3 tmp = {difference.y * move_sensitivity, -difference.x * move_sensitivity, 0};
				// view_position.y -= difference.x * move_sensitivity;
				// view_position.x += difference.y * move_sensitivity;
				glm_vec3_rotate(tmp, -pitch * 2, (vec3){1, 0, 0});
				glm_vec3_rotate(tmp, yaw, (vec3){0, 1, 0});
				glm_vec3_add(view_position.v, tmp, view_position.v);
			}else{

				yaw += difference.y * sensitivity;
				pitch += difference.x * sensitivity;

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
				SDL_WarpMouseInWindow(window, SCREEN_WIDTH - 11, mouse_pos.y);
			}
			if(mouse_pos.x >= SCREEN_WIDTH - 10){
				SDL_WarpMouseInWindow(window, 11, mouse_pos.y);
				SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
				current_pos = (Vector2){mouse_pos.x, mouse_pos.y};
				// last_pos = (Vector2)current_pos;
			}
			last_pos = (Vector2)current_pos;
		}
	// }
}