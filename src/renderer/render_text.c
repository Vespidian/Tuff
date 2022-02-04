#include "../global.h"
#include "../debug.h"
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
    default_font = *NewRawFont("default_font", "../bin/builtin_assets/default_font.png", GL_RGBA, (Vector2){6, 12}, (Vector2){5, 2});
    DebugLog(D_ACT, "Initialized font subsystem");
}

FontObject *NewFont(char *name, TilesheetObject *tilesheet, Vector2 char_size, Vector2 padding){
	FontObject *return_font = &default_font;
	// Resize 'font_stack' to fit new font
	FontObject *tmp_font_stack = realloc(font_stack, sizeof(FontObject) * (num_fonts + 1));
	if(tmp_font_stack != NULL){
		font_stack = tmp_font_stack;

		// Allocate memory for font name
		font_stack[num_fonts].name = malloc(strlen(name) + 1);
		strcpy(font_stack[num_fonts].name, name);
		font_stack[num_fonts].name[strlen(name)] = 0;

		// Copy data to font
		font_stack[num_fonts] = (FontObject){.id = nextID, *tilesheet, char_size, padding};
		DebugLog(D_ACT, "Created font id '%d' name '%s'", nextID, name);

		return_font = &font_stack[num_fonts];

		// Increment stack counters
		nextID++;
		num_fonts++;
	}else{
		DebugLog(D_WARN, "%s: Creating new font: Could not reallocate font stack", name);
	}
    return return_font;
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

void FreeFontStack(){
	for(int i = 0; i < num_fonts; i++){
		free(font_stack[i].name);
		font_stack[i].name = NULL;
	}
	free(font_stack);
	font_stack = NULL;
}

void RenderText(FontObject *font, float font_size, int x_pos, int y_pos, int alignment, char *text, ...){
    if(text != NULL){
		va_list va_format;
		va_list copy;
		
		//Use var args to create formatted text
		va_start(va_format, text);
		va_copy(copy, va_format);
		int length = vsnprintf(NULL, 0, text, copy);
		va_end(copy);
		char *formatted_text = NULL;
		formatted_text = malloc(length + 1);
		if(formatted_text != NULL){
			vsnprintf(formatted_text, length + 1, text, va_format);
			formatted_text[length] = 0;

			RenderTextEx(font, font_size, x_pos, y_pos, (Vector4){1, 1, 1, 1}, alignment, RNDR_TEXT, -1, formatted_text);
			
			free(formatted_text);
		}
		va_end(va_format);
	}
}

void RenderTextEx(FontObject *font, float font_size, int x_pos, int y_pos, Vector4 color, int alignment, int render_layer, int num_characters, char *text, ...){
    if(font == NULL){
		return;
	}
	char *formattedText = NULL;
	int string_length = num_characters;
	va_list va_format;
	if(num_characters != -1){
		va_list copy;

		// Convert va_arg to single string
		va_start(va_format, text);
		va_copy(copy, va_format);
		string_length = vsnprintf(NULL, 0, text, copy);
		va_end(copy);
		formattedText = malloc(string_length + 1);
	}else{
		string_length = strlen(text);
		formattedText = text;
	}

	if(formattedText != NULL){
		// // Check if va_arg (-1) or normal formatted text was inputted
		if(num_characters != -1){
			vsnprintf(formattedText, string_length, text, va_format);
			formattedText[string_length] = 0;
		}

		// Set up dst character rect
		SDL_Rect dst = {x_pos, y_pos, font->tilesheet.tile_w * font_size, font->tilesheet.tile_h * font_size};
		int char_value = 0;

		// Loop through each character of string and render them
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
				RenderTilesheet(font->tilesheet, char_value, &dst, render_layer, color);
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
		if(num_characters == -1){
			formattedText = NULL;
		}
	}
	if(num_characters == -1){
    	va_end(va_format);
	}
}