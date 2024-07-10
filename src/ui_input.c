#include <stdbool.h>
#include <SDL2/SDL.h>

#include "vectorlib.h"
#include "event.h"

#include "ui.h"

static bool hold = false;
static iVector2 holdpos;
static iVector2 diff;

static void SliderFunc(UIState *state, UIElement *element, UI_MOUSE_EVENT events){
	if((events & UI_MOUSE_CLICK) != 0){
		hold = true;
		holdpos = mouse_pos;
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	if((events & UI_MOUSE_RELEASE) != 0){
		hold = false;
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}

	if(hold){
		diff.x = 0;
		diff.y = 0;
		if((mouse_pos.x != holdpos.x) || (mouse_pos.y != holdpos.y)){
			diff.x = mouse_pos.x - holdpos.x;
			diff.y = mouse_pos.y - holdpos.y;

			extern SDL_Window *window;
			SDL_WarpMouseInWindow(window, holdpos.x, holdpos.y);

		}
		state->focused_element = element;
		
		element->slider.val += diff.x * element->slider.step;
		if(element->slider.val > element->slider.max){
			element->slider.val = element->slider.max;
		}
		if(element->slider.val < element->slider.min){
			element->slider.val = element->slider.min;
		}
		
        if(element->slider.modify_width){
		    element->offset.z = element->slider.val;
        }

		int len = snprintf(NULL, 0, "%f", element->slider.val);
		if(len < 64){
			snprintf(element->text, len, "%f", element->slider.val);
		}
	}
}

void UISliderNew(UIElement *element, float min, float max, float val_default, float step){
	if((element != NULL) && (element->input_type == UI_INPUT_NONE)){
		element->input_type = UI_INPUT_SLIDER;
		element->slider.min = min;
		element->slider.max = max;
		element->slider.step = step;
		element->slider.val = val_default;
		element->slider.func = SliderFunc;

        element->slider.modify_width = false;

		element->class.size_min.x = 1;

		free(element->text);
		element->text = malloc(sizeof(char) * 64);
	}

}




void UIToggleNew(UIElement *element, bool init_state){
    if((element != NULL) && (element->input_type == UI_INPUT_NONE)){

    }
}