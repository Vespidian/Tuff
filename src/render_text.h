#ifndef RENDER_TEXT_H_
#define RENDER_TEXT_H_

enum TEXT_ALIGN{TEXT_ALIGN_LEFT = 0, TEXT_ALIGN_RIGHT, TEXT_ALIGN_CENTER};

/**
 *  Font struct containing the data of a font
 */
typedef struct FontObject{
    char *name;
    unsigned int id;
    Texture texture;
    iVector2 tile_size;
    iVector2 char_size;
    iVector2 padding;
}FontObject;

/**
 *  Font used in all cases unless specified otherwise
 */
extern FontObject default_font;

/**
 *  Initialize the text rendering subsystem
 */
void InitFonts();

void FreeFontStack();

/**
 *  @brief generate a new font (to be used when rendering text)
 *  @param name name of the font
 *  @param tilesheet tilesheet containing font characters
 *  @param tile_size size of tiles tilesheet is divided into
 *  @param char_size size of the boundaries of all characters within font
 *  @param padding area around character in tilesheet
 *  @return A pointer to the font in the font stack
 */
FontObject NewFont(char *name, Texture texture, iVector2 tile_size, iVector2 char_size, iVector2 padding);

// /**
//  *  @brief generate a new font directly from a texture on the disk (to be used when rendering text)
//  *  @param name name of the font
//  *  @param path path to texture file on disk
//  *  @param image_format format of tilesheet image (GL_RGB, RL_RGBA)
//  *  @param char_size size of the boundaries of all characters within font
//  *  @param padding area around character in tilesheet
//  *  @return A pointer to the font in the font stack
//  */
// FontObject *NewRawFont(char *name, char *path, int image_format, Vector2 char_size, Vector2 padding);

/**
 *  @return Font with name 'name' from the font stack
 */
FontObject *FindFont(char *name);

/**
 *  @return Font with id 'id' from the font stack
 */
FontObject *IDFindFont(unsigned int id);

/**
 *  @brief Render text to the screen
 *  @param font font to be used when rendering
 *  @param font_size scale factor of text
 *  @param x_pos/y_pos position to render the text within the window
 *  @param text formatted text to be rendered
 *  @param ... information to be placed in format specifiers
 */
void RenderText(FontObject *font, float font_size, int x_pos, int y_pos, int alignment, char *text, ...);

/**
 *  @brief Render text to the screen
 *  @param font font to be used when rendering
 *  @param font_size scale factor of text
 *  @param x_pos/y_pos position to render the text within the window
 *  @param color color to tint text (font must be white to begin with)
 *  @param num_characters va_arg specifier (-1 specifies va_arg text input)
 *  @param text formatted text to be rendered
 *  @param ... information to be placed in format specifiers
 */
void RenderTextEx(FontObject *font, float font_size, int x_pos, int y_pos, Vector4 color, int alignment, int render_layer, int num_characters, char *text, ...);

#endif