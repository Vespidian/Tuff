#ifndef TILESHEET_H_
#define TILESHEET_H_

#include "../textures.h"

/**
 *  Array of tilesheets loaded via 'NewTilesheet()'
 */
extern TilesheetObject *tilesheet_stack;

/**
 *  Tilesheet to be used in error cases
 */
extern TilesheetObject undefined_tilesheet;

/**
 *  Initialize the tilesheet subsystem
 */
void InitTilesheets();

/**
 *  Generate a tilesheet from a preloaded texture
 *  @param texture previously loaded texture to be used as tilesheet texture
 *  @param tile_. width and height of a single tile
 *  @return A pointer to the 'TilesheetObject' containing the tilesheet
 */
TilesheetObject *NewTilesheet(Texture texture, int tile_w, int tile_h);

/**
 *  Generate a tilesheet from from a texture file
 *  @param texture texture to be loaded and used as tilesheet texture
 *  @param tile_. width and height of a tile
 *  @return A pointer to the 'TilesheetObject' containing the tilesheet
 */
TilesheetObject *NewTilesheetFromFile(char *path, int format, int tile_w, int tile_h);

/**
 *  @return A pointer to the tilesheet with id 'id' from tilesheet stack
 */
TilesheetObject *FindTilesheet(unsigned int id);

#endif