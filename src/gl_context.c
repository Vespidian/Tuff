#include <math.h>
#include "global.h"
#include "gl_utils.h"

#include "event.h"
#include "debug.h"
#include "renderer/renderer.h"
#include "renderer/quad.h"
#include "ui/ui.h"

#include "gl_context.h"


#include "scene/obj_loader.h"

SDL_GLContext gl_context;

mat4 projection_matrix;
int z_depth = 1000;

TilesheetObject tilesheet_texture;
TextureObject crate_tex;

static void WindowResize(EventData event);
static void KeyPresses(EventData event);
static void Zoom(EventData event);
static void MouseEvent(EventData event);

//testing
mat4 perspective_projection;
mat4 view_matrix;
unsigned int mesh_vbo;
unsigned int mesh_vao;
unsigned int axis_vao;
unsigned int axis_vbo;
unsigned int mesh_shader;
unsigned int grid_shader;
MeshObject mesh;
MeshObject axis_mesh;
mat4 mesh_matrix;
mat4 axis_matrix;

float data[] = {
	// vx, vy, vz, tu, tv, tw, nx, ny, nz,
	// -1.000000, 1.000000, 1.000000,
	// -1.000000, -1.000000, -1.000000,
	// -1.000000, -1.000000, 1.000000,

	// -1.000000, 1.000000, -1.000000,
	// 1.000000, -1.000000, -1.000000,
	// -1.000000, -1.000000, -1.000000,
	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
};

void ReloadObj(){
	FreeObj(&mesh);
	// LoadObj("../models/crate.obj", &mesh);
}

int InitGL(){
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	// SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

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
    // glEnable(GL_CULL_FACE);
	// glEnable(GL_MULTISAMPLE);
	// glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.5f, 0.4f, 0.3f, 1);
	glm_ortho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, -z_depth / 2, z_depth / 2, projection_matrix);

	BindEvent(EV_ACCURATE, SDL_WINDOWEVENT, WindowResize);
    BindEvent(EV_QUICK, SDL_KEYDOWN, KeyPresses);
    BindEvent(EV_ACCURATE, SDL_MOUSEWHEEL, Zoom);
    BindEvent(EV_ACCURATE, SDL_MOUSEMOTION, MouseEvent);
    BindEvent(EV_ACCURATE, SDL_MOUSEBUTTONDOWN, MouseEvent);

	InitGLUtils();

	RendererInit();


    tilesheet_texture = LoadTilesheet("../images/testingTemp/tmpTilesheet.png", 16, 16);
	crate_tex = LoadTexture("../images/crate.png");


    DebugLog(D_ACT, "Initialized OpenGL");


	mesh_shader = LoadShaderProgram("../shaders/mesh.vert", "../shaders/mesh.frag");
	grid_shader = LoadShaderProgram("../shaders/mesh.vert", "../shaders/grid.frag");

	BindKeyEvent(ReloadObj, 'r', SDL_KEYDOWN);
	LoadObj("../models/axis.obj", &axis_mesh);
	// LoadObj("../models/cube.obj", &mesh);
	// LoadObj("../models/monkey.obj", &mesh);
	// LoadObj("../models/3d_arrow.obj", &mesh);
	LoadObj("../models/polygons.obj", &mesh);
	// LoadObj("../models/camera.obj", &mesh);
	// LoadObj("../models/multi_mesh.obj", &mesh);
	// LoadObj("../models/mug.obj", &mesh);
	printf("vertices: %d\n", mesh.num_vertices);
	printf("faces: %d\n", mesh.num_faces);
	// LoadObj("../models/cube.obj");


	// Generate vertex array (to be able to store mesh render settings)
	glGenVertexArrays(1, &mesh_vao);
	glBindVertexArray(mesh_vao);

	// Generate vertex buffer (to store mesh data)
	glGenBuffers(1, &mesh_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * num_f, mesh.data, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.data_size, mesh.data, GL_STREAM_DRAW);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 3));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 6));



	glGenVertexArrays(1, &axis_vao);
	glBindVertexArray(axis_vao);

	// Generate vertex buffer (to store mesh data)
	glGenBuffers(1, &axis_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * axis_mesh.data_size, axis_mesh.data, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 3));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 6));


    glm_mat4_identity(perspective_projection);
	glm_mat4_identity(mesh_matrix);
    glm_mat4_identity(view_matrix);

    // set model transform
    glm_translate(mesh_matrix, (vec3){0, 0, -1.0});
    // glm_rotate(mesh_matrix, glm_rad(-55), (vec3){1.0, 0.0, 0.0});
    glm_scale(mesh_matrix, (vec3){0.5, 0.5, 0.5});
    // glm_scale(mesh_matrix, (vec3){0.1, 0.1, 0.1});

    // set view transform
    // glm_translate(view_matrix, (vec3){0, 0, 1.0f});

    // set projection transform
    glm_perspective(glm_rad(90), SCREEN_WIDTH / SCREEN_HEIGHT, 0.01, 100, perspective_projection);
    // glm_ortho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, -1, 1, projection);

    UniformSetMat4(mesh_shader, "projection", perspective_projection);
    UniformSetMat4(mesh_shader, "model", mesh_matrix);
    UniformSetMat4(mesh_shader, "view", view_matrix);

    UniformSetMat4(grid_shader, "projection", perspective_projection);
    UniformSetMat4(grid_shader, "model", mesh_matrix);
    UniformSetMat4(grid_shader, "view", view_matrix);

	UniformSetInt(mesh_shader, "tex", 0);


	// for(int i = 0; i < 3 * 3 * 3 * 64; i++){
	// // for(int i = 0; i < 3; i++){
	// 	// if(i % 3 == 0){
	// 	// 	printf("\n");
	// 	// }
	// 	if(i % 9 == 0){
	// 		printf("\n");
	// 	}
	// 	// if(data[i] != vert_buffer[i]){
	// 	// 	// printf("%f, ", data[i]);
	// 	// 	// printf("%f, ", vert_buffer[i]);
	// 	// 	// printf("!\n");
	// 		printf("%f /", vert_buffer[i]);
	// 	// }
	// }

	// vert_buffer[0] = 0;
	// printf("%f\n", vert_buffer[0]);

    GLCall;
    return 0;
}


// static float value = 0;
static Vector3 direction = {0, 0, 0};
static Vector3 view_position = {0, 0, 0};
static Vector3 mesh_position = {0, 0.5, 0};
static float view_distance = 5;
static float yaw, pitch;
#include "ui/elements/slider.h"
#include "ui/ui.h"
#include "renderer/render_text.h"
void RenderUI(){
	Vector2 size = {200, 30};
	Vector2 origin = {SCREEN_WIDTH - size.x / 2 - 10, size.y * 3};
	Vector2 origin2 = {SCREEN_WIDTH - size.x / 2 - 10, size.y * 3};
	RenderSlider(&yaw, 0.0, M_PI * 2, VerticalRectList_vec(3, 0, size, origin, 6));
	RenderSlider(&pitch, 0.0, M_PI * 2, VerticalRectList_vec(3, 1, size, origin, 6));
	RenderSlider(&direction.z, 0.0, M_PI * 2, VerticalRectList_vec(3, 2, size, origin, 6));
	RenderTextEx(&default_font, 1, VerticalRectList_vec(3, 0, size, origin2, 6).x, VerticalRectList_vec(3, 0, size, origin2, 6).y, (Vector4){1, 0, 0, 1}, TEXT_ALIGN_RIGHT, -1, "x: ");
	RenderTextEx(&default_font, 1, VerticalRectList_vec(3, 1, size, origin2, 6).x, VerticalRectList_vec(3, 1, size, origin2, 6).y, (Vector4){0, 1, 0, 1}, TEXT_ALIGN_RIGHT, -1, "y: ");
	RenderTextEx(&default_font, 1, VerticalRectList_vec(3, 2, size, origin2, 6).x, VerticalRectList_vec(3, 2, size, origin2, 6).y, (Vector4){0, 0, 1, 1}, TEXT_ALIGN_RIGHT, -1, "z: ");


    glm_mat4_identity(view_matrix);
	glm_mat4_identity(mesh_matrix);
	glm_mat4_identity(axis_matrix);

	
	glm_translate(view_matrix, (vec3){0, 0, -view_distance});
	glm_rotate(view_matrix, pitch * 2, (vec3){1, 0, 0});
	glm_rotate(view_matrix, yaw, (vec3){0, 1, 0});
	glm_translate(view_matrix, view_position.v);
	

	UniformSetMat4(mesh_shader, "view", view_matrix);
	UniformSetMat4(grid_shader, "view", view_matrix);
	RenderText(&default_font, 1, 0, 0, TEXT_ALIGN_LEFT, "view_distance: %f", view_distance);


    glm_translate(mesh_matrix, mesh_position.v);
	glm_scale_uni(mesh_matrix, 0.5f);

    UniformSetMat4(mesh_shader, "model", mesh_matrix);

	glm_scale_uni(axis_matrix, 50.0f);
	UniformSetMat4(grid_shader, "model", axis_matrix);

    // // GLCall;
}
void RenderGL(){
	RenderUI();
	SetShaderProgram(mesh_shader);

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, crate_tex.gl_tex);
	current_texture_unit = 0;
	bound_textures[0] = crate_tex.gl_tex;

	// glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	glBindVertexArray(mesh_vao);
	current_vao = mesh_vao;
	glDrawArrays(GL_TRIANGLES, 0, mesh.num_vertices);

	// glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 192, mesh.data, GL_STATIC_DRAW);
	// glDrawArrays(GL_TRIANGLES, 0, 192);

	// glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.data_size, mesh.data, GL_STATIC_DRAW);

	// glBufferData(GL_ARRAY_BUFFER, sizeof(float) * axis_mesh.data_size, axis_mesh.data, GL_STATIC_DRAW);
	// glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);
	// glBindVertexArray(mesh_vao);
	SetShaderProgram(grid_shader);
	glBindVertexArray(axis_vao);
	current_vao = axis_vao;
	glDrawArrays(GL_TRIANGLES, 0, axis_mesh.num_vertices);

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
		glm_ortho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, -z_depth / 2, z_depth / 2, projection_matrix);
    	// glm_mat4_identity(perspective_projection);
		// printf("width: %d\nheight: %d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
		glm_perspective(glm_rad(90), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01, 100, perspective_projection);
		UniformSetMat4(mesh_shader, "projection", perspective_projection);
		UniformSetMat4(grid_shader, "projection", perspective_projection);
	}
}

Vector2 last_pos;
Vector2 current_pos;
static float sensitivity = 0.01f;
static float move_sensitivity = 0.01f;
static void MouseEvent(EventData event){
	current_pos = (Vector2){mouse_pos.y, mouse_pos.x};
	if(mouse_clicked){
		last_pos = (Vector2)current_pos;
	}
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
			// direction.x += difference.x * sensitivity;
			// direction.y += difference.y * sensitivity;
			yaw += difference.y * sensitivity;
			pitch += difference.x * sensitivity;
			// if(direction.x > M_PI * 2){
			// 	direction.x -= M_PI * 2;
			// }
			// if(direction.x < 0){
			// 	direction.x += M_PI * 2;
			// }

			// if(direction.y > M_PI * 2){
			// 	direction.y -= M_PI * 2;
			// }
			// if(direction.y < 0){
			// 	direction.y += M_PI * 2;
			// }
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
			// yaw += difference.x * sensitivity;
			// pitch += difference.y * sensitivity;
			// if(direction.x > 360){
			// 	direction.x -= 360;
			// }
			// if(direction.x < 0){
			// 	direction.x += 360;
			// }

			// if(direction.y > 360){
			// 	direction.y -= 360;
			// }
			// if(direction.y < 0){
			// 	direction.y += 360;
			// }
			// direction.x = cos(yaw) * cos(pitch);
			// direction.y = sin(pitch);
			// direction.z = sin(yaw) * cos(pitch);
			// float distance = 10.0;
			// direction.x = (distance * cos(yaw)) * cos(pitch);
			// direction.y = distance * sin(pitch);
			// direction.z = (distance * sin(yaw)) * cos(pitch);
			// glm_normalize(direction.v);
			direction.x = yaw;
			direction.y = pitch;
		}

		last_pos = (Vector2)current_pos;
	}
}