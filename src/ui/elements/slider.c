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
	SDL_Rect handle = {map(*current, min, max, transform.x, transform.x + transform.z) - 6, transform.y + transform.w / 8, 12, transform.w - transform.w / 4};
	SDL_Rect rect;
	if(handle.x - (transform.x - padding) > 6){
		rect = (SDL_Rect){transform.x + padding, transform.y + padding, handle.x - transform.x, transform.w - padding * 2};
		ResizableRect(ui_tilesheet, rect, 13, 6, RNDR_UI + 1, (Vector4){0.69, 0.83, 0.93, 1});
	}
	rect = (SDL_Rect){transform.x - padding, transform.y, transform.z + padding * 2, transform.w};
	RenderText(&default_font, 1, transform.x + transform.z / 2, transform.y + transform.w / 4, TEXT_ALIGN_CENTER, "%.2f", *current);
	ResizableRect(ui_tilesheet, handle, 13, 6, RNDR_UI + 2, (Vector4){1, 1, 1, 1});
	ResizableRect(ui_tilesheet, rect, 13, 6, RNDR_UI, (Vector4){0, 0, 0, 1});

	rect = (SDL_Rect){transform.x, transform.y, transform.z, transform.w};
	if(SDL_PointInRect(&mouse_pos, &rect) && mouse_held){
		ui_hovered = true;
		handle.x = mouse_pos.x;
		*current = map(handle.x, transform.x + transform.z, transform.x, max, min);
	}
}