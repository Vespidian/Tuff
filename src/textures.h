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
 *  Array of textures loaded via 'LoadTextureToStack()'
 */
// extern Texture *texture_stack;

/**
 *  Texture to be used in error cases
 */
extern Texture undefined_texture;

/**
 *  @brief Initialize the texture subsystem
 */
void InitTextures();

/**
 *  @brief Load a texture from file and place it into the texture stack
 *  @return A pointer to the 'Texture' containing the texture
 */
// Texture *LoadTextureToStack(const char *path);

/**
 *  @return A pointer to the texture with id 'id' from the texture stack
 */
Texture *TextureFind(unsigned int texture);

Texture TextureOpen(char *path);
void TextureFree(Texture *texture);
TilesheetObject LoadTilesheet(char *path, int tile_width, int tile_height);

#endif