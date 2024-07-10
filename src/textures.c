#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <GL/glew.h>
#include <SDL2/SDL_image.h>

#include "debug.h"

#include "textures.h"

#ifdef DEBUG_BUILD
#define GLCall CheckGLErrors(__FILE__, __LINE__);
#else
#define GLCall
#endif


extern Texture undefined_texture;

// void InitTextures(){
//     undefined_texture = TextureOpen("../bin/builtin_assets/undefined.png", TEXTURE_FILTERING_NEAREST);
//     DebugLog(D_ACT, "Initialized texture subsystem");
// }

void TextureFree(Texture *texture){
	if(texture != NULL){
		#ifndef NOOPENGL
			unsigned int *id = &texture->gl_tex;
			glDeleteTextures(1, id);
		#endif
		texture->is_loaded = false;
		texture->w = 0;
		texture->h = 0;

		free(texture->path);
		texture->path = NULL;
	}
}

static int InvertSurfaceVertical(SDL_Surface *surface)
{
    Uint8 *t;
    register Uint8 *a, *b;
    Uint8 *last;
    register Uint16 pitch;

    /* do nothing unless at least two lines */
    if(surface->h < 2) {
        return 0;
    }

    /* get a place to store a line */
    pitch = surface->pitch;
    t = (Uint8*)malloc(pitch);

    if(t == NULL) {
        // SDL_UNLOCKIFMUST(surface);
        return -2;
    }

    /* get first line; it's about to be trampled */
    memcpy(t,surface->pixels,pitch);

    /* now, shuffle the rest so it's almost correct */
    a = (Uint8*)surface->pixels;
    last = a + pitch * (surface->h - 1);
    b = last;

    while(a < b) {
        memcpy(a,b,pitch);
        a += pitch;
        memcpy(b,a,pitch);
        b -= pitch;
    }

    /* in this shuffled state, the bottom slice is too far down */
    memmove( b, b+pitch, last-b );
    /* now we can put back that first row--in the last place */
    memcpy(last,t,pitch);

    free(t);

    return 0;
}

Texture TextureOpen(char *path, TEXTURE_FILTERING_TYPE filtering){
    Texture texture = undefined_texture;
	texture.filtering = filtering;

	if(path != NULL){
		texture.path = malloc(strlen(path) + 1);
		memcpy(texture.path, path, strlen(path));
		texture.path[strlen(path)] = 0;

		
		SDL_Surface *tmp_surface = NULL;
		unsigned int gl_texture;

		GLCall(glGenTextures(1, &gl_texture));
		GLCall(glBindTexture(GL_TEXTURE_2D, gl_texture));

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		if(filtering == TEXTURE_FILTERING_LINEAR){
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		}else{
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		}

		tmp_surface = IMG_Load(texture.path);
		if(tmp_surface == NULL){
			printf("%s: Error opening image\n", texture.path);
			DebugLog(D_ERR, "%s: Error opening image\n", texture.path);
			return undefined_texture;
		}
		InvertSurfaceVertical(tmp_surface);

		texture.gl_tex = gl_texture;

		texture.w = tmp_surface->w;
		texture.h = tmp_surface->h;
		if(tmp_surface != NULL){
			// TODO: Support more image formats / image color formats without crashing
			int internal_format;
			int image_format;
			if(SDL_ISPIXELFORMAT_ALPHA(tmp_surface->format->format)){
				internal_format = GL_RGBA8;
				image_format = GL_RGBA;
			}else{
				internal_format = GL_RGB8;
				image_format = GL_RGB;
			}
			// switch(tmp_surface->format->format){
			// 	case 
			// }
			// // if(format == GL_RGB){
			// 	internal = GL_RGB8;
			// // }
			// printf("name: %s --- format: %s\n", path, SDL_GetPixelFormatName(tmp_surface->format->format));
			// int internal = GL_RGBA8;
			// if(format == GL_RGB){
			// 	internal = GL_RGB8;
			// }
			// GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tmp_surface->w, tmp_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp_surface->pixels));
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, tmp_surface->w, tmp_surface->h, 0, image_format, GL_UNSIGNED_BYTE, tmp_surface->pixels));
			GLCall(glGenerateMipmap(GL_TEXTURE_2D));

			DebugLog(D_ACT, "Loaded texture: '%s'", texture.path);
			texture.is_loaded = true;
		}else{
			DebugLog(D_ERR, "Could not load image: '%s'", texture.path);
			texture = undefined_texture;

		}
		SDL_FreeSurface(tmp_surface);
	}
    return texture;
}

void TextureReload(Texture *texture){
	if(texture != NULL){
		TEXTURE_FILTERING_TYPE filtering = texture->filtering;
		char *path = malloc(strlen(texture->path) + 1);
		memcpy(path, texture->path, strlen(texture->path));
		path[strlen(texture->path)] = 0;

		TextureFree(texture);
		*texture = TextureOpen(path, filtering);

		free(path);
	}
}
