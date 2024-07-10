#include <stdbool.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <cglm/cglm.h>

#include "debug.h"
#include "vectorlib.h"
#include "event.h"
#include "engine.h"

int SCREEN_WIDTH =  800;
int SCREEN_HEIGHT =  800;
float z_depth = 1000.0;

SDL_Window *window = NULL;
SDL_GLContext gl_context;
extern bool running;

unsigned int uniform_buffer;

mat4 perspective_projection;
mat4 orthographic_projection;
mat4 view_matrix;
unsigned int engine_time = 0;



void MessageCallback(
	GLenum source, 
	GLenum type, 
	GLuint id, 
	GLenum severity, 
	GLsizei length, 
	const GLchar* message, 
	const void* userParam){
	
	printf("------ OpenGL Callback: ------ \nid:%d\ntype:", id);
	switch(type){
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			printf("DEPRECATED BEHAVIOUR\n");
			break;
		case GL_DEBUG_TYPE_ERROR:
			printf("ERROR\n");
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			printf("UNDEFINED_BEHAVIOUR\n");
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			printf("PORTABILITY\n");
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			printf("PERFORMANCE\n");
			break;
		case GL_DEBUG_TYPE_OTHER:
			printf("OTHER\n");
			break;
	}
	printf("severity:");
	switch (severity){
		case GL_DEBUG_SEVERITY_LOW:
			printf("LOW");
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			printf("MEDIUM");
			break;
		case GL_DEBUG_SEVERITY_HIGH:
			printf("HIGH");
			break;
    }
	printf("\nmessage: %s\n", message);
}


static void UpdateUniforms(){
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	
	// perspective projection matrix
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16 * 0, sizeof(float) * 16, &perspective_projection);
	
	// orthographics projection matrix
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16 * 1, sizeof(float) * 16, &orthographic_projection);

	// view matrix (camera)
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16 * 2, sizeof(float) * 16, &view_matrix);

	// engine_time
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 16 * 3, sizeof(unsigned int), &engine_time);
}

static void WindowResize(EventData event){
	if(event.e->window.event == SDL_WINDOWEVENT_RESIZED){
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glm_perspective(glm_rad(90), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01, 1000, perspective_projection);
		glm_ortho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, -z_depth / 2, z_depth / 2, orthographic_projection);

		UpdateUniforms();
	}
}


void UpdateGL(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO: possibly only update uniforms that change every frame here
	UpdateUniforms();
}

static void InitGLState(){
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
	glEnable(GL_PROGRAM_POINT_SIZE);
    // glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	// glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.258, 0.258, 0.258, 1);
	// glEnable(GL_DEBUG_OUTPUT);
	// glDebugMessageCallback(MessageCallback, 0);
}

static void InitGlobalUniforms(){
	// Generate the uniform buffer for global things like time and certain matrices
	glGenBuffers(1, &uniform_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * (16 * 3 + 4), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);

	// Calculate the matrices matrix
	glm_perspective(glm_rad(90), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.01, 1000, perspective_projection);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glm_ortho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, -z_depth / 2, z_depth / 2, orthographic_projection);
	
	// Initialize camera view matrix
	glm_mat4_identity(view_matrix);
	glm_translate(view_matrix, (vec3){0, 0, 0});

	// Send global uniform data to GPU
	UpdateUniforms();
}

static void InitGL(){
	InitGLState();

	InitGlobalUniforms();	
}

void InitSDL(){
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		DebugLog(D_ERR, "Could not initialize SDL. SDL_Error: %s", SDL_GetError());
		return;
	}
	window = SDL_CreateWindow("Tuff", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if(window == NULL){
		DebugLog(D_ERR, "SDL window could not be created. SDL_Error: %s", SDL_GetError());
		return;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1);

    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        DebugLog(D_ERR, "Failed to initialize GLEW!");
        return;
    }
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	running = true;

	// Function to update window size variables and render matrices if the window is resized
	BindEvent(EV_POLL_ACCURATE, SDL_WINDOWEVENT, WindowResize);

	// Initialize GL state
    InitGL();
}