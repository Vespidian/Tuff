#include <string.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdarg.h>

#include "renderer/quad.h"
#include "debug.h"

#include "render_text.h"

FontObject default_font;

// Number of fonts in the 'font_stack'
unsigned int num_fonts = 0;

void InitFonts(){
    default_font = NewFont("default_font", TextureOpen("../bin/builtin_assets/default_font.png", TEXTURE_FILTERING_NEAREST), (iVector2){16, 16}, (iVector2){6, 12}, (iVector2){5, 2});
    DebugLog(D_ACT, "Initialized font subsystem");
}

FontObject NewFont(char *name, Texture texture, iVector2 tile_size, iVector2 char_size, iVector2 padding){
	FontObject font = default_font;

	if(name != NULL){
		// Allocate memory for font name
		font.name = malloc(strlen(name) + 1);
		strcpy(font.name, name);
		font.name[strlen(name)] = 0;
	}

	// Copy data to font
	font = (FontObject){.id = num_fonts++, texture, tile_size, char_size, padding};
	DebugLog(D_ACT, "Created font id '%d' name '%s'", num_fonts - 1, name);

    return font;
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

			RenderTextEx(font, font_size, x_pos, y_pos, (Vector4){1, 1, 1, 1}, alignment, 0, -1, formatted_text);
			
			free(formatted_text);
			formatted_text = NULL;
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
		Vector4 dst = {x_pos, y_pos, font->tile_size.x * font_size, font->tile_size.y * font_size};
		int char_value = 0;

		// Loop through each character of string and render them
		for(int i = 0; i < string_length; i++){
			// Adjust character value so that space is 0 (everything before space is unprintable / not a visual character)
			char_value = (int)formattedText[i] - (int)' ';

			// Check if character is a printable character
			if(char_value > 0){
				// switch(alignment){
				// 	case TEXT_ALIGN_CENTER:
				// 		if(i < string_length / 2){
				// 			dst.x = x_pos - (string_length / 2.0 - i) * ((float)font->padding.x * 1.7f * font_size) - ((float)font->padding.x * 1.7f * font_size) / 2;
				// 		}else{
				// 			dst.x = x_pos + (i - string_length / 2.0) * ((float)font->padding.x * 1.7f * font_size) - ((float)font->padding.x * 1.7f * font_size) / 2;
				// 		}
				// 		break;
				// 	case TEXT_ALIGN_RIGHT:
				// 		dst.x = x_pos - (string_length - i) * ((float)font->padding.x * 1.7f * font_size);
				// 		break;
				// 	default:
				// 		dst.x = x_pos + i * ((float)font->padding.x * 1.7f * font_size);
				// 		break;
				// }
				RenderTilesheet(font->texture, char_value, font->tile_size, &dst, render_layer, color);

				dst.x += (float)font->padding.x * 1.7f * font_size;

			}else{// Some unprintable characters do stuff
				switch(char_value){
					case 0:
						dst.x += (float)font->padding.x * 1.7f * font_size;
						break;
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