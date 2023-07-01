#include <unistd.h>

#include "global.h"
#include "event.h"
#include "gl_context.h"
#include "textures.h"
#include "debug.h"


#include "renderer/tilesheet.h"
#include "renderer/renderer.h"
#include "renderer/render_text.h"
#include "renderer/quad.h"

#include "ui/ui.h"
#include "ui/ui_utility.h"
#include "ui/ui_layout.h"
#include "ui/ui_parser.h"

#include "json_base.h"
#include "bundle.h"

int loop_start_ticks = 0;
float deltatime = 0;
int target_framerate = 120;

int SCREEN_WIDTH =  800;
int SCREEN_HEIGHT =  800;
SDL_Window *window = NULL;

bool running = false;
float time_scale = 1.0f;
bool paused = false;
bool main_menu = true;
bool window_active = true;

// Possible name for the project: Pallet

Bundle app;

void InitSDL(){
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		DebugLog(D_ERR, "Could not initialize SDL. SDL_Error: %s", SDL_GetError());
		return;
	}
	window = SDL_CreateWindow("FoXandbox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
}

void InitRenderers(){
	InitQuadRender();
}

// static char *startup_dict[] = {
// 	"startup_bundle",
// 	NULL
// };
// char *startup_path = NULL;
// static void tfunc_startup(JSONState *json, unsigned int token){
// 	switch(JSONTokenHash(json, token, startup_dict)){
// 		case 0:; // startup_bundle
// 			JSONTokenToString(json, token + 1, &startup_path);
// 			if(startup_path != NULL){
// 				if(strncmp(startup_path + strlen(startup_path) - 5, ".bndl", 5) == 0){ // Make sure the file is a '.bndl' file
// 					int start_time = SDL_GetTicks();
// 					app = BundleOpen(startup_path);
// 					printf("Loaded bundle.. Took %dms\n", SDL_GetTicks() - start_time);
// 				}
// 			}
// 			break;
// 		default:
// 			break;
// 	}
// }

void Setup(){
	// JSONSetErrorFunc(DebugLog);
	InitDebug();

	if(chdir("../assets") != 0){
		perror("Could not set working directory");
	}

	InitEvents();

	#ifndef NOOPENGL
		InitSDL();
	#endif
	InitUndefined();

	app = BundleOpen(NULL);
	// int timer = SDL_GetTicks();
	// JSONState json = JSONOpen("../bin/builtin_assets/startup.conf");
	// JSONSetTokenFunc(&json, NULL, tfunc_startup);
	// JSONParse(&json);
	// JSONFree(&json);
	// printf("Loaded bundle in '%d' ms\n", SDL_GetTicks() - timer);

	#ifndef NOOPENGL
		InitGL();
	#endif

	// InitRenderers();
	// InitTilesheets();
	// InitFonts();

}

#include "scene.h"
extern Model model;
extern Model model2;
extern Model light_model;

void Quit(){
	DebugLog(D_ACT, "Shutting down!");
	running = false;

	ModelFree(&model);
	ModelFree(&model2);
	ModelFree(&light_model);

	BundleFree(&app);

	// Eventually unnecessary
	{
		ExitGL();
		ShaderFree(&quad_shader);
		FreeUndefined();
		// free(startup_path);
		// startup_path = NULL;
	}
	RendererQuit();
	
	QuitEvents();
	SDL_Quit();
	QuitDebug();
}

static void CheckWindowActive(EventData event){
	if(event.e->window.event == SDL_WINDOWEVENT_FOCUS_LOST){
		window_active = false;
	}else if(event.e->window.event == SDL_WINDOWEVENT_FOCUS_GAINED){
		window_active = true;
	}
}

// extern Texture crate_tex;
// extern Texture normal_map;
static void ReloadApp(){
	DebugLog(D_ACT, "\n\nReloading Bundle\n\n");
	int timer = SDL_GetTicks();
	// BundleFree(&app);
	// app = BundleOpen(startup_path);

	// // ShaderFree(&axis_shader);
	// // axis_shader = ShaderOpen("shaders/axis.shader");
	// // ShaderFree(&mesh_shader);
	// // mesh_shader = ShaderOpen("shaders/mesh.shader");

	BundleReload(&app);
	printf("Loaded bundle in '%d' ms\n", SDL_GetTicks() - timer);

	// model.material = BundleMaterialFind(&app, "materials/default.mat");
	// model.bundle = &app;
	// // model.mesh = &BundleGLTFFind(&app, "models/soda_can.gltf")->meshes[0];
	// // model.mesh = &undefined_mesh;
	// ModelSetMesh(&model, &BundleGLTFFind(&app, "models/soda_can.gltf")->meshes[0]);

	// model2.material = BundleMaterialFind(&app, "materials/squish.mat");
	// // model2.mesh = &BundleGLTFFind(&app, "models/squish_cube.gltf")->meshes[0];
	// model2.bundle = &app;
	// model2.mesh = &undefined_gltf.meshes[0];

	// light_model.material = BundleMaterialFind(&app, "materials/light.mat");
	// light_model.bundle = &app;
	// light_model.mesh = &BundleGLTFFind(&app, "meshes/sphere.gltf")->meshes[0];
	
}

bool wireframe = false;
static void ToggleWireframe(EventData event){
	if(event.keyStates[SDL_SCANCODE_LSHIFT]){
		wireframe = !wireframe;
		if(wireframe){
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}else{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
}

#include "ui/ui_utility.h"

int main(int argc, char *argv[]){
	Setup();

	BindKeyEvent(ToggleWireframe, 'z', SDL_KEYDOWN);
	BindKeyEvent(ReloadApp, 'm', SDL_KEYDOWN);
	BindEvent(EV_POLL_ACCURATE, SDL_WINDOWEVENT, CheckWindowActive);

#ifndef NOOPENGL
	while(running){
#endif
		loop_start_ticks = SDL_GetTicks();
		if(window_active){
			#ifndef NOOPENGL
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				RenderGL();
			#endif

			PushRender();
			
			#ifndef NOOPENGL
				SDL_GL_SwapWindow(window);
			#endif
		}
		EventListener();
		
		SDL_Delay(1000 / target_framerate);
		deltatime = (SDL_GetTicks() - loop_start_ticks) / 10.0;
#ifndef NOOPENGL
	}
#else
	// ReloadApp();
	// model = ModelNew(NULL, &undefined_mesh, &undefined_material);
	// ModelSetMesh(&model, &BundleGLTFFind(&app, "models/soda_ca.gltf")->meshes[0]);
	// ModelFree(&model);
	// &BundleGLTFFind(&app, "models/soda_can.gltf")->meshes[0];


	// model = ModelNew(NULL, &BundleGLTFFind(&app, "models/soda_can.gltf")->meshes[0], BundleMaterialFind(&app, "materials/default.mat"));
	// model2 = ModelNew(NULL, &BundleGLTFFind(&app, "models/squish_cube.gltf")->meshes[0], BundleMaterialFind(&app, "materials/squish.mat"));
	// light_model = ModelNew(NULL, &BundleGLTFFind(&app, "models/uv_sphere.gltf")->meshes[0], BundleMaterialFind(&app, "materials/light.mat"));

	model = ModelNew(NULL, &BundleGLTFFind(&app, "models/entrance.gltf", true)->meshes[0], BundleMaterialFind(&app, "materials/default.mat", true));
	model2 = ModelNew(NULL, &BundleGLTFFind(&app, "models/squish_cube.gltf", true)->meshes[0], BundleMaterialFind(&app, "materials/squish.mat", true));
	light_model = ModelNew(NULL, &BundleGLTFFind(&app, "models/uv_sphere.gltf", true)->meshes[0], BundleMaterialFind(&app, "materials/light.mat", true));


	// model = ModelNew(NULL, &BundleGLTFFind(&app, "models/soda_can.gltf")->meshes[0], &app.materials[0]);
	// BundleMaterialFind(&app, "materials/squish.mat");
	// model2 = ModelNew(NULL, &BundleGLTFFind(&app, "models/squish_cube.gltf")->meshes[0], &app.materials[1]);
	// BundleMaterialFind(&app, "materials/light.mat");
	// light_model = ModelNew(NULL, &BundleGLTFFind(&app, "models/uv_sphere.gltf")->meshes[0], &app.materials[2]);


#endif

	Quit();
	return 0;
}