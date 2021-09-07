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
#include "ui/ui_layout.h"
#include "ui/ui_parser.h"


#include "gyro/gyro.h"


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

TilesheetObject builtin_tilesheet;
void LoadBuiltinResources(){
	builtin_tilesheet = LoadTilesheet("../images/builtin.png", 16, 16);
}

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

void Setup(){
	InitDebug();
	InitEvents();
	InitSDL();
	InitGL();

	InitRenderers();

	InitTextures();
	InitTilesheets();
	InitFonts();

	// NewScene("../ui/barebones.uiss");
	NewScene("../ui/test.ui");

	LoadBuiltinResources();


	InitGyro();
}

void Quit(){
	DebugLog(D_ACT, "Shutting down!");
	// UnloadSandbox();
	running = false;
	
	SDL_Quit();
	QuitDebug();
}

void GameLoop(){

	
	
	RenderText(&default_font, 1, SCREEN_WIDTH - 10, 10, TEXT_ALIGN_RIGHT, "Number of render appends: %d", num_append_instance_calls);


}

static void CheckWindowActive(EventData event){
	if(event.e->window.event == SDL_WINDOWEVENT_FOCUS_LOST){
		window_active = false;
	}else if(event.e->window.event == SDL_WINDOWEVENT_FOCUS_GAINED){
		window_active = true;
	}
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
#include "scene/obj_loader.h"
int main(int argc, char *argv[]){
	Setup();
	// startupTime.x = SDL_GetTicks();
	BindKeyEvent(ToggleWireframe, 'z', SDL_KEYDOWN);
	BindEvent(EV_POLL_ACCURATE, SDL_WINDOWEVENT, CheckWindowActive);

	// LoadObj("../models/cube.obj");

	while(running){
		loop_start_ticks = SDL_GetTicks();
		EventListener();
		if(window_active){
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			RenderGL();
			LoopGyro();
			// if(main_menu){
				// RenderStartScreen();
			// }else{
			// 	GameLoop();
			// }
			PushRender();
			SDL_GL_SwapWindow(window);
		}
		
		SDL_Delay(1000 / target_framerate);
		deltatime = (SDL_GetTicks() - loop_start_ticks) / 10.0;
	}
	Quit();
	return 0;
}