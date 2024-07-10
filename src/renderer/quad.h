#ifndef QUAD_H_
#define QUAD_H_

#include "../shader.h"
#include "../textures.h"

/**
 *  Initialize the quad render system
 */
void InitQuadRender();

extern Shader quad_shader;

/**
 *  Render a quad with a texture to the viewport
 */
void RenderQuad(Texture texture, Vector4 *src, Vector4 *dst, int zpos, Vector4 color, float rot);

/**
 *  Render a tile from a tilesheet to the viewport
 */
void RenderTilesheet(Texture texture, unsigned int index, iVector2 tile_size, Vector4 *dst, int zpos, Vector4 color);


#endif