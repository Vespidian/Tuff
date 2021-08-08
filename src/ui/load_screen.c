#include "../global.h"
#include "../gl_utils.h"
#include "../renderer/quad.h"
#include "../event.h"
#include "ui.h"

#include "load_screen.h"

#define LOADBARSIZE SCREEN_WIDTH / 3
bool loading = true;

int loadAmount = 0;
int duration = 40000;

TextureObject loadscreen_texture;

void LoadScreenInit(){
	loadscreen_texture = LoadTexture("../images/loadScreen.png");
}

void LoadScreen(){
	if(loadAmount == LOADBARSIZE){
		loading = false;
		enable_input = true;
	}
	if(loading){
		enable_input = false;
		RenderQuad(loadscreen_texture, NULL, &(SDL_Rect){SCREEN_WIDTH / 2 - 128, SCREEN_HEIGHT / 2 - 128, 256, 256}, RNDR_TEXT + 2, (Vector4){1, 1, 1, 1}, 0);
		RenderTilesheet(builtin_tilesheet, 0, &(SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, RNDR_TEXT + 1, (Vector4){0.12, 0.63, 0.56, 1});
		loadAmount += IntegerLerp(loadAmount, LOADBARSIZE, 65536 - duration);
		RenderTilesheet(builtin_tilesheet, 0, &(SDL_Rect){SCREEN_WIDTH / 2 - LOADBARSIZE / 2, SCREEN_HEIGHT / 2 + 150, loadAmount, 20}, RNDR_TEXT + 1, (Vector4){0.62, 0.11, 0.18, 1});
	}
}