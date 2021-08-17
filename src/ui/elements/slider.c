#include "../../global.h"
#include "../resizable_rect.h"
#include "../../renderer/render_text.h"
#include "../ui.h"
#include "../../event.h"

/**
 *  How to allow for the mouse cursor to leave the slider rect but keep interacting with the slider:
 *  Create an internal array of sliders each frame
 *  Each slider will a boolean to specify whether its selected or not
 *  Sliders have an 'id' (order in which they are rendered / created)
 *  id specified their index in the slider array
 */

float map(float x, float in_min, float in_max, float out_min, float out_max){
	return (x - in_max) * (out_max - out_min) / (in_min - in_max) + out_min;
}

int padding = 6;

void RenderSlider(float *current, float max, float min, Vector4_i transform){
	SDL_Rect rect;
	rect = (SDL_Rect){transform.x - padding, transform.y, transform.z + padding * 2, transform.w};
	RenderText(&default_font, 1, transform.x + transform.z / 2, transform.y + transform.w / 4, TEXT_ALIGN_CENTER, "%.2f", *current);
	ResizableRect(ui_tilesheet, rect, 13, 6, RNDR_UI, (Vector4){0, 0, 0, 1});

	rect = (SDL_Rect){transform.x, transform.y, transform.z, transform.w};

	if(SDL_PointInRect(&mouse_pos, &rect)){
		ui_hovered = true;
		if(mouse_clicked){
			mouse_pos.x = (transform.x + transform.z / 2.0);
		}
		// SDL_SetRelativeMouseMode(SDL_TRUE);
		// 	SDL_SetWindowGrab(window, SDL_TRUE);
		// SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
		if(mouse_held && !mouse_clicked){
			ui_selected = true;
			SDL_WarpMouseInWindow(window, transform.x + transform.z / 2, transform.y + transform.w / 2);
			*current += ((transform.x + transform.z / 2.0) - mouse_pos.x) / 40;
		}
		*current += scroll_value * ((max - min) / 20.0);
	}
	if(mouse_lifted){
		ui_selected = false;
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
}