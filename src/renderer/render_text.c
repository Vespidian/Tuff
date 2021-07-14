#include "../global.h"
#include "../debug.h"
#include "../utility.h"
#include "../gl_utils.h"
#include "quad.h"

#include "render_text.h"

FontObject default_font;

FontObject *font_stack;

// Number of fonts in the 'font_stack'
unsigned int num_fonts = 0;

// ID to be used by next font created
static unsigned int nextID = 0;

void InitFonts(){
    default_font = *NewRawFont("default_font", "../images/fonts/default_font.png", GL_RGBA, (Vector2){6, 12}, (Vector2){5, 2});
    DebugLog(D_ACT, "Initialized font subsystem");
}

FontObject *NewFont(char *name, TilesheetObject *tilesheet, Vector2 char_size, Vector2 padding){
	// Resize 'font_stack' to fit new font
    font_stack = realloc(font_stack, sizeof(FontObject) * (num_fonts + 1));

	// Allocate memory for font name
    font_stack[num_fonts].name = malloc(sizeof(char) * strlen(name));

	// Copy data to font
    font_stack[num_fonts] = (FontObject){name, nextID, *tilesheet, char_size, padding};
    DebugLog(D_ACT, "Created font id '%d' name '%s'", nextID, name);

	// Increment stack counters
    nextID++;
    return &font_stack[num_fonts++];
}

FontObject *NewRawFont(char *name, char *path, int image_format, Vector2 char_size, Vector2 padding){
    return NewFont(name, NewTilesheetFromFile(path, image_format, char_size.x + padding.x * 2, char_size.y + padding.y * 2), char_size, padding);
}

FontObject *FindFont(char *name){
    for(int i = 0; i < num_fonts; i++){
        if(strcmp(font_stack[i].name, name) == 0){
            return &font_stack[i];
        }
    }
    return &default_font;
}

FontObject *IDFindFont(unsigned int id){
    for(int i = 0; i < num_fonts; i++){
        if(font_stack[i].id == id){
            return &font_stack[i];
        }
    }
    return &default_font;
}

void RenderText(FontObject *font, float font_size, int x_pos, int y_pos, int alignment, char *text, ...){
    va_list va_format;

    va_start(va_format, text);
    char formattedText[256];
    vsprintf(formattedText, text, va_format);
    va_end(va_format);

    RenderTextEx(font, font_size, x_pos, y_pos, (Vector4){1, 1, 1, 1}, alignment, -1, formattedText);
}

void RenderTextEx(FontObject *font, float font_size, int x_pos, int y_pos, Vector4 color, int alignment, int num_characters, char *text, ...){
    va_list va_format;

	// Convert va_arg to single string
    va_start(va_format, text);
    char formattedText[256];
    vsprintf(formattedText, text, va_format);
    va_end(va_format);

	// Check if va_arg (-1) or normal formatted text was inputted
    if(num_characters != -1){
        formattedText[num_characters] = '\0';
    }

	// Set up dst character rect
    SDL_Rect dst = {x_pos, y_pos, font->tilesheet.tile_w * font_size, font->tilesheet.tile_h * font_size};
    int char_value = 0;

	// Loop through each character of string and render them
	int string_length = strlen(formattedText); 
    for(int i = 0; i < string_length; i++){
		// Adjust character value so that space is 0 (everything before space is unprintable / not a visual character)
        char_value = (int)formattedText[i] - (int)' ';

		// Check if character is a printable character
        if(char_value >= 0){
			switch(alignment){
				case TEXT_ALIGN_CENTER:
					if(i < string_length / 2){
						dst.x = x_pos - (string_length / 2.0 - i) * ((float)font->padding.x * 1.7f * font_size) - ((float)font->padding.x * 1.7f * font_size) / 2;
					}else{
            			dst.x = x_pos + (i - string_length / 2.0) * ((float)font->padding.x * 1.7f * font_size) - ((float)font->padding.x * 1.7f * font_size) / 2;
					}
					break;
				case TEXT_ALIGN_RIGHT:
            		dst.x = x_pos - (string_length - i) * ((float)font->padding.x * 1.7f * font_size);
					break;
				default:
            		dst.x = x_pos + i * ((float)font->padding.x * 1.7f * font_size);
					break;
			}
            RenderTilesheet(font->tilesheet, char_value, &dst, RNDR_TEXT, color);
        }else{// Some unprintable characters do stuff
            switch(char_value){
                case -22: // NEWLINE (\n)

					// Reset the x position and increment the y position
                    dst.y += (font->char_size.y + font->padding.y) * font_size;
                    dst.x = x_pos;
                    break;
                case -23: // TAB

					// Increment the x position
                    dst.x += 8 * font_size;
                    break;
            }
        }
    }
}