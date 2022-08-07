#ifndef TEXTURES_H_
#define TEXTURES_H_

typedef enum TEXTURE_FILTERING_TYPE{TEXTURE_FILTERING_NEAREST, TEXTURE_FILTERING_LINEAR, TEXTURE_FILTERING_BILINEAR, TEXTURE_FILTERING_TRILINEAR, TEXTURE_FILTERING_ANISOTROPIC} TEXTURE_FILTERING_TYPE;

typedef struct Texture{
    unsigned int gl_tex;
	char *path;
	bool is_loaded;
	TEXTURE_FILTERING_TYPE filtering;
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
 *  @brief Initialize the texture subsystem
 */
void InitTextures();

/**
 * 
 */
Texture TextureNew();

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
void TextureReload(Texture *texture);

/**
 * 
 */
TilesheetObject LoadTilesheet(char *path, int tile_width, int tile_height);

#endif