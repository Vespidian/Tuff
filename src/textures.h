#ifndef TEXTURES_H_
#define TEXTURES_H_

typedef struct Texture{
    unsigned int gl_tex;
	char *path;
	bool is_loaded;
    int w;
    int h;
}Texture;

typedef struct TilesheetObject{
	unsigned int id;
    Texture texture;
    int tile_w;
    int tile_h;
}TilesheetObject;

/**
 *  Texture to be used in error cases
 */
extern Texture undefined_texture;

/**
 *  @brief Initialize the texture subsystem
 */
void InitTextures();

/**
 * 
 */
Texture TextureOpen(char *path);

/**
 * 
 */
void TextureFree(Texture *texture);

/**
 * 
 */
TilesheetObject LoadTilesheet(char *path, int tile_width, int tile_height);

#endif