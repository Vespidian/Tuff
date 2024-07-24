#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <stdbool.h>
#include <unistd.h>
#include <cglm/cglm.h>

#include "vectorlib.h"
#include "debug.h"
#include "sdl_gl_init.h"
#include "engine.h"
#include "event.h"
#include "textures.h"
#include "render_text.h"
#include "renderer/quad.h"

bool running = false;

void Init();
void Quit();

extern void EngineSetup();
extern void EngineExit();
extern void EngineLoop();

extern SDL_Event e;
int main(int argc, char *argv[]){
    Init();

    while(running){
        EventListener();

        UpdateGL();

        EngineLoop();

        SDL_GL_SwapWindow(window);

		engine_time++;

        SDL_Delay(10);
    }

    Quit();

    return 0;
}

void Init(){
    InitDebug();
    InitEvents();
    InitSDL();
	InitTextures();

    if(chdir("../assets") != 0){
		perror("Could not set working directory");
	}

	InitFonts();
	InitQuadRender();

    EngineSetup();
    printf("Hello world!!\n");
}

void Quit(){
    running = false;
    EngineExit();

    QuitEvents();
    SDL_Quit();
    QuitDebug();
}
