#include <unistd.h>

#include "global.h"
#include "event.h"
#include "gl_context.h"
#include "textures.h"
#include "debug.h"


#include "renderer/tilesheet.h"
#include "renderer/renderer.h"
#include "renderer/render_text.h"
#include "renderer/render_text.h"
#include "renderer/quad.h"
#include "renderer/materials/mask.h"

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


Bundle app;
// TilesheetObject builtin_tilesheet;
// void LoadBuiltinResources(){
// 	builtin_tilesheet = LoadTilesheet("../images/builtin.png", 16, 16);
// }

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
	// InitUI();
	
}

// void LoadDiffDomain(char *path){
// 	printf("Loading: %s\n", path);
// 	UI_LoadDomain(path);
// }

static char *startup_dict[] = {
	"startup_bundle",
	NULL
};
char *startup_path = NULL;
static void tfunc_startup(JSONState *json, unsigned int token){
	switch(JSONTokenHash(json, token, startup_dict)){
		case 0:; // startup_bundle
			JSONTokenToString(json, token + 1, &startup_path);
			if(startup_path != NULL){
				if(strncmp(startup_path + strlen(startup_path) - 5, ".bndl", 5) == 0){ // Make sure the file is a '.bndl' file
					int start_time = SDL_GetTicks();
					app = BundleOpen(startup_path);
					printf("Loaded bundle.. Took %dms\n", SDL_GetTicks() - start_time);
				}
			}
			break;
		default:
			break;
	}
}

void Setup(){
	// JSONSetErrorFunc(DebugLog);
	InitDebug();

	if(chdir("../assets") != 0){
		perror("Could not set working directory");
	}

	InitEvents();
	InitSDL();


	JSONState json = JSONOpen("../bin/builtin_assets/startup.conf");
	JSONSetTokenFunc(&json, NULL, tfunc_startup);
	JSONParse(&json);
	JSONFree(&json);


	InitGL();
	InitRenderers();
	InitTextures();
	InitTilesheets();
	InitFonts();

	// Load startup file

	// int start = SDL_GetTicks();
	// UI_LoadDomain("ui/minimal.uiss");
	// UI_LoadDomain("ui/tests/color.uiss");
	// FindElement(&scene_stack[0], "color")->function = script_onclick;
	// FindElement(&scene_stack[0], "layout")->function = script_onclick;

	// printf("Loading ui took: %dms\n", SDL_GetTicks() - start);
}

void Quit(){
	DebugLog(D_ACT, "Shutting down!");
	// UnloadSandbox();
	running = false;
	BundleFree(&app);
	QuitEvents();
	SDL_Quit();
	QuitDebug();
}

void GameLoop(){

	
	
	// RenderText(&default_font, 1, SCREEN_WIDTH - 10, 10, TEXT_ALIGN_RIGHT, "Number of render appends: %d", num_append_instance_calls);


}

static void CheckWindowActive(EventData event){
	if(event.e->window.event == SDL_WINDOWEVENT_FOCUS_LOST){
		window_active = false;
	}else if(event.e->window.event == SDL_WINDOWEVENT_FOCUS_GAINED){
		window_active = true;
	}
}

static void ReloadApp(EventData event){
	BundleFree(&app);
	app = BundleOpen(startup_path);

	ShaderFree(&axis_shader);
	axis_shader = ShaderOpen("shaders/axis.shader");
}

static void ReloadUI(){
	// for(int i = 0; i < num_domains; i++){
	// 	UI_FreeDomain(&scene_stack[i]);
	// }
	printf("BYUN DOMAINS: %d\n", num_domains);
	// UI_LoadDomain("ui/minimal.uiss");
	// UI_LoadDomain("ui/blenderish.uiss");
	// UI_LoadDomain("ui/tests/color.uiss");
	// FindElement(&scene_stack[0], "color")->function = script_onclick;
	// FindElement(&scene_stack[0], "layout")->function = script_onclick;
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

	// ShaderOpen_new("shaders/default.shader");

	// startupTime.x = SDL_GetTicks();
	BindKeyEvent(ToggleWireframe, 'z', SDL_KEYDOWN);
	BindKeyEvent(ReloadUI, 'i', SDL_KEYDOWN);
	BindKeyEvent(ReloadApp, 'm', SDL_KEYDOWN);
	// BindKeyEvent(ReloadScript, 'l', SDL_KEYDOWN);
	BindEvent(EV_POLL_ACCURATE, SDL_WINDOWEVENT, CheckWindowActive);

	// LoadObj("../models/cube.obj");
	// UI_LoadDomain("ui/tests/color.uiss");

	while(running){
		loop_start_ticks = SDL_GetTicks();
		EventListener();
		if(window_active){
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// if(script_loop != NULL){
			// 	script_loop();
			// }

			RenderGL();

			// SetElementText(FindElement(&scene_stack[0], "t1"), "Time: %d", SDL_GetTicks());+
			// for(int i = 0; i < num_domains; i++){
			// 	UI_RenderDomain(&scene_stack[i]);

			// }
			// UI_RenderDomain(&scene_stack[1]);
			PushRender();
			SDL_GL_SwapWindow(window);
		}
		
		SDL_Delay(1000 / target_framerate);
		deltatime = (SDL_GetTicks() - loop_start_ticks) / 10.0;
	}
	Quit();
	return 0;
}