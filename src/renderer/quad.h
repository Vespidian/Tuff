#ifndef QUAD_H_
#define QUAD_H_

#include "../textures.h"

/**
 *  Initialize the quad render system
 */
void InitQuadRender();

extern Shader quad_shader;

/**
 *  Render a quad with a texture to the viewport
 */
void RenderQuad(Texture texture, SDL_Rect *src, SDL_Rect *dst, int zpos, Vector4 color, float rot);

/**
 *  Render a tile from a tilesheet to the viewport
 */
void RenderTilesheet(TilesheetObject tilesheet, unsigned int index, SDL_Rect *dst, int zpos, Vector4 color);

#endif