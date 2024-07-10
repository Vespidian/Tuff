#ifndef TEXTURES_H_
#define TEXTURES_H_

typedef enum TEXTURE_FILTERING_TYPE{TEXTURE_FILTERING_NEAREST, TEXTURE_FILTERING_LINEAR} TEXTURE_FILTERING_TYPE;

typedef struct Texture{
    unsigned int gl_tex;
	char *path;
	bool is_loaded;
    TEXTURE_FILTERING_TYPE filtering;
    int w;
    int h;
}Texture;

/**
 *  Texture to be used in error cases
 */
// extern Texture undefined_texture;

/**
 * Initialize the texture subsystem
 */
// void InitTextures();

/**
 * Opens the texture at location 'path'. If it cannot be
 * found, 'undefined_texture' is returned
*/
Texture TextureOpen(char *path, TEXTURE_FILTERING_TYPE filtering);

/**
 * Frees texture data from memory
 */
void TextureFree(Texture *texture);

/**
 *  Attempts to load the texture from the file it was initially loaded from
 */
void TextureReload(Texture *texture);

#endif
