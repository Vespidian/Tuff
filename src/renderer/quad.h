#ifndef QUAD_H_
#define QUAD_H_

/**
 *  Initialize the quad render system
 */
void InitQuadRender();

/**
 *  Render a quad with a texture to the viewport
 */
void RenderQuad(TextureObject texture, SDL_Rect *src, SDL_Rect *dst, int zpos, Vector4 color, float rot);

/**
 *  Render a tile from a tilesheet to the viewport
 */
void RenderTilesheet(TilesheetObject tilesheet, unsigned int index, SDL_Rect *dst, int zpos, Vector4 color);

#endif