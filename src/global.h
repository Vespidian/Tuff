#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>

#include "include/cglm/cglm.h"
#include "gl_utils.h"

#define DEBUG_BUILD

enum RENDER_LAYERS {RNDR_BACKGROUND = 0, RNDR_LEVEL = 5, RNDR_ENTITY = 20, RNDR_UI = 30, RNDR_TEXT = 40};

typedef struct Vector2_i{
	int x;
	int y;
}Vector2_i;


typedef struct {
	union{
		struct {float x, y;};
		float v[2];
	};
}Vector2;

typedef struct {
	union{
		struct {float r, g, b;};
		struct {float x, y, z;};
		struct {float s, t, p;};
		float v[3];
	};
}Vector3;

typedef struct {
	union{
		struct {float r, g, b, a;};
		struct {float x, y, z, w;};
		struct {float s, t, p, q;};
		float v[4];
	};
}Vector4;

typedef struct {
	union{
		struct {int r, g, b, a;};
		struct {int x, y, z, w;};
		struct {int s, t, p, q;};
		int v[4];
	};
}Vector4_i;


extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern SDL_Window *window;

extern bool running;
extern float deltatime;
extern int target_framerate;

void Quit();

//tmp
extern TilesheetObject builtin_tilesheet;

#endif