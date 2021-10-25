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


int loop_start_ticks = 0;
float deltatime = 0;
int target_framerate = 120;

int SCREEN_WIDTH =  800;
int SCREEN_HEIGHT =  800;
SDL_Window *window = NULL;

bool running = true;
float time_scale = 1.0f;
bool paused = false;
bool main_menu = true;
bool window_active = true;

// TilesheetObject builtin_tilesheet;
// void LoadBuiltinResources(){
// 	builtin_tilesheet = LoadTilesheet("../images/builtin.png", 16, 16);
// }

void InitSDL(){
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
		DebugLog(D_ERR, "Could not initialize SDL. SDL_Error: %s", SDL_GetError());
	}
	window = SDL_CreateWindow("FoXandbox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if(window == NULL){
		DebugLog(D_ERR, "SDL window could not be created. SDL_Error: %s", SDL_GetError());
	}
}

void InitRenderers(){
	InitQuadRender();
	InitUI();
	
}

void LoadDiffDomain(char *path){
	printf("Loading: %s\n", path);
	UI_LoadDomain(path);
}

// #include <libtcc.h>
// void (*script_setup)() = NULL;
// void (*script_loop)() = NULL;
// void (*script_onclick)(UIElement *element) = NULL;
// TCCState *script;

// void TccDebug(void *opaque, const char *msg){
// 	DebugLog(D_ERR, "%s", msg);
// }

// struct Object{
// 	int d1;
// 	float f1;
// };

void Setup(){
	InitDebug();
	InitEvents();
	InitSDL();
	InitGL();

	InitRenderers();

	InitTextures();
	InitTilesheets();
	InitFonts();

	int start = SDL_GetTicks();
	UI_LoadDomain("../ui/minimal.uiss");
	UI_LoadDomain("../ui/tests/color.uiss");
	// FindElement(&scene_stack[0], "color")->function = script_onclick;
	// FindElement(&scene_stack[0], "layout")->function = script_onclick;

	printf("Loading ui took: %dms\n", SDL_GetTicks() - start);

		/*start = SDL_GetTicks();
		struct Object prnt = {56, 1.48f};

		// Create a script object
		script = tcc_new();
		if(!script){
			printf("Couldnt initialize tcc!\n");
		}

		// Make sure we compile to memory (must be set before compilation)
		tcc_set_output_type(script, TCC_OUTPUT_MEMORY);
		
		tcc_set_options(script, "g b Wall");
		tcc_set_error_func(script, NULL, TccDebug);
		// tcc_add_sysinclude_path(script, "include");
		// printf("%d\n", script->do_bounds_check);
		// script->do_bounds_check = true;
		// script->do_debugging = true;

		// Load the source file
		if(tcc_add_file(script, "../resources/script.c") == -1){
			printf("Couldnt load script file\n");
		}

		Vector3 pos = {5.6, 7.89, 2.731};
		Vector3 clr = {5, 8, 2};
		tcc_add_symbol(script, "parent", &prnt);
		tcc_add_symbol(script, "position", &pos);
		tcc_add_symbol(script, "color", &clr);
		tcc_add_symbol(script, "LoadDomain", &LoadDiffDomain);

		if(tcc_relocate(script, TCC_RELOCATE_AUTO) == -1){
			printf("Error relocating script code\n");
		}else{
			// Successfully compiled! Now we can get the functions we need


			script_setup = tcc_get_symbol(script, "Setup");
			script_loop = tcc_get_symbol(script, "Loop");
			script_onclick = tcc_get_symbol(script, "OnClick");

			if(script_setup != NULL){
				script_setup();
			}
		}
		printf("Loading and compiling c took: %dms\n", SDL_GetTicks() - start);*/


	// LoadBuiltinResources();
}

void Quit(){
	DebugLog(D_ACT, "Shutting down!");
	// UnloadSandbox();
	running = false;
	
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

/*static void ReloadScript(){
	int start = SDL_GetTicks();

	tcc_delete(script);
	struct Object prnt = {56, 1.48f};

		// Create a script object
		script = tcc_new();
		if(!script){
			printf("Couldnt initialize tcc!\n");
		}

		// Make sure we compile to memory (must be set before compilation)
		tcc_set_output_type(script, TCC_OUTPUT_MEMORY);
		
		tcc_set_options(script, "g b Wall");
		tcc_set_error_func(script, NULL, TccDebug);
		// tcc_add_sysinclude_path(script, "include");
		// printf("%d\n", script->do_bounds_check);
		// script->do_bounds_check = true;
		// script->do_debugging = true;

		// Load the source file
		if(tcc_add_file(script, "../resources/script.c") == -1){
			printf("Couldnt load script file\n");
		}


		if(tcc_relocate(script, TCC_RELOCATE_AUTO) == -1){
			printf("Error relocating script code\n");
		}else{
			// Successfully compiled! Now we can get the functions we need

		Vector3 pos = {5.6, 7.89, 2.731};
		Vector3 clr = {5, 8, 2};
		tcc_add_symbol(script, "parent", &prnt);
		tcc_add_symbol(script, "position", &pos);
		tcc_add_symbol(script, "color", &clr);

			script_setup = tcc_get_symbol(script, "Setup");
			script_loop = tcc_get_symbol(script, "Loop");
			script_onclick = tcc_get_symbol(script, "OnClick");

			if(script_setup != NULL){
				script_setup();
			}
		}
		printf("Loading and compiling c took: %dms\n", SDL_GetTicks() - start);

}*/

static void ReloadUI(){
	for(int i = 0; i < num_domains; i++){
		UI_FreeDomain(&scene_stack[i]);
	}
	UI_LoadDomain("../ui/minimal.uiss");
	// UI_LoadDomain("../ui/tests/color.uiss");
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
	// startupTime.x = SDL_GetTicks();
	BindKeyEvent(ToggleWireframe, 'z', SDL_KEYDOWN);
	BindKeyEvent(ReloadUI, 'i', SDL_KEYDOWN);
	// BindKeyEvent(ReloadScript, 'l', SDL_KEYDOWN);
	BindEvent(EV_POLL_ACCURATE, SDL_WINDOWEVENT, CheckWindowActive);

	// LoadObj("../models/cube.obj");

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
			for(int i = 0; i < num_domains; i++){
				UI_RenderDomain(&scene_stack[i]);

			}
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