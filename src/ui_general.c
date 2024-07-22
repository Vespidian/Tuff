#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "vectorlib.h"
#include "debug.h"
#include "ui.h"

extern Texture default_texture;

UIState UINewState(){
	UIState state;
	state.path = NULL;
	state.num_classes = 0;
	state.num_elements = 0;
	state.focused_element = NULL;
	return state;
}

UIClass UIDefaultEmptyClass(){
	UIClass c;

	c.id = -1;
	c.name = NULL;
	c.offset = (iVector2){2147483647, 2147483647};

	c.size_min = (iVector2){-1, -1};
	c.size_max = (iVector2){-1, -1};
	c.size_min_percent = (Vector2){-1, -1};
	c.size_max_percent = (Vector2){-1, -1};

	c.padding = (iVector4){-1, -1, -1, -1};
	c.border = (iVector4){-1, -1, -1, -1};
	c.margin = (iVector4){-1, -1, -1, -1};
	c.color = (Vector3){-1, -1, -1};
	c.border_color = (Vector3){-1, -1, -1};

	c.wrap = -1;
	c.wrap_vertical = -1;
	c.wrap_reverse = -1;
	c.origin_p = UI_ORIGIN_UNDEFINED;
	c.origin_c = UI_ORIGIN_UNDEFINED;

	c.culling = -1;
	c.inherit = false;

	/** Mouse Events **/
	c.class_hold = NULL;
	c.class_hover = NULL;

	c.event_func = NULL;

	return c;
}

UIClass *UINewClass(UIState *state){
	UIClass *c = &state->classes[0];
	if((state != NULL) && (state->num_classes < UI_STATE_MAX_CLASSES)){
		c = &state->classes[state->num_classes];
		*c = UIDefaultEmptyClass();
		c->id = state->num_classes++;
	}else{
		DebugLog(D_ERR, "%s:%s: error: Could not create new class: UI_STATE_MAX_CLASSES reached", __FILE__, __LINE__);
	}

	return c;
}

UIClass UIDefaultElementClass(){
	UIClass c;

	c.id = -1;
	c.name = NULL;
	c.offset = (iVector2){0, 0};

	c.size_min = (iVector2){100, 100};
	c.size_max = (iVector2){200, 200};
	c.size_min_percent = (Vector2){-1, -1};
	c.size_max_percent = (Vector2){-1, -1};

	c.padding = (iVector4){10, 10, 10, 10};
	c.border = (iVector4){1, 1, 1, 1};
	c.margin = (iVector4){10, 10, 10, 10};
	c.color = (Vector3){1, 1, 1};
	c.border_color = (Vector3){0, 0, 0};

	c.wrap = true;
	c.wrap_vertical = false;
	c.wrap_reverse = false;
	c.origin_p = UI_ORIGIN_NORTHWEST;
	c.origin_c = UI_ORIGIN_NORTHWEST;

	c.culling = false;
	c.inherit = false;

	/** Mouse Events **/
	c.class_hold = NULL;
	c.class_hover = NULL;

	c.event_func = NULL;
	
	return c;
}

UIClass UIDefaultRootClass(){
	UIClass c;

	c.id = -1;
	c.name = NULL;
	c.offset = (iVector2){0, 0};

	c.size_min = (iVector2){100, 100};
	c.size_max = (iVector2){200, 200};
	c.size_min_percent = (Vector2){-1, -1};
	c.size_max_percent = (Vector2){-1, -1};

	c.padding = (iVector4){0, 0, 0, 0};
	c.border = (iVector4){0, 0, 0, 0};
	c.margin = (iVector4){0, 0, 0, 0};
	c.color = (Vector3){1, 1, 1};
	c.border_color = (Vector3){0, 0, 0};

	c.wrap = true;
	c.wrap_vertical = false;
	c.wrap_reverse = false;
	c.origin_p = UI_ORIGIN_NORTHWEST;
	c.origin_c = UI_ORIGIN_NORTHWEST;

	c.culling = false;
	c.inherit = false;

	/** Mouse Events **/
	c.class_hold = NULL;
	c.class_hover = NULL;

	c.event_func = NULL;
	
	return c;
}

UIElement *UINewElement(UIState *state){
	UIElement *e = &state->elements[0];

	if((state != NULL) && (state->num_elements < UI_STATE_MAX_ELEMENTS)){
		e = &state->elements[state->num_elements];

		e->id = state->num_elements++;

		e->name = NULL;
		e->text = NULL;

		e->num_children = 0;
		e->children = NULL;

		e->num_classes = 0;
		e->classes = NULL;
		
		e->num_tmp_classes = 0;
		e->tmp_classes = NULL;

		e->transform = (iVector4){0, 0, 100, 100};
		e->offset = (iVector4){0, 0, 0, 0};
		
		e->texture = default_texture;

		e->mouse_events = 0;
		e->visible = true;
		e->visible_children = true;
		e->input_type = UI_INPUT_NONE;

		e->class = UIDefaultEmptyClass();
		e->style = UIDefaultElementClass();
		e->event_func = NULL;

		e->parent = NULL;
	}else{
		DebugLog(D_ERR, "%s:%s: error: Could not create new class: UI_STATE_MAX_ELEMENTS reached", __FILE__, __LINE__);
	}

	return e;
}



static void UIFreeElement(UIElement *element){
	free(element->name);
	element->name = NULL;
	
	free(element->text);
	element->text = NULL;

	free(element->children);
	element->children = NULL;
	element->num_children = 0;
	
	free(element->classes);
	element->classes = NULL;
	element->num_classes = 0;

	free(element->tmp_classes);
	element->tmp_classes = NULL;
	element->num_tmp_classes = 0;
}

void UIFreeState(UIState *state){
	if(state != NULL){
		for(int i = 0; i < state->num_elements; i++){
			UIFreeElement(&state->elements[i]);
		}
		state->num_elements = 0;

		for(int i = 0; i < state->num_classes; i++){
			free(state->classes[i].name);
			free(state->classes[i].class_hold);
			free(state->classes[i].class_hover);
		}
		state->num_classes = 0;
	}
}



void UIElementAddChild(UIElement *parent, UIElement *child){
	// Now to set up the parent
	child->parent = parent;
	if((parent != NULL) && (child != NULL)){
		UIElement **tmp = realloc(parent->children, sizeof(UIElement *) * (parent->num_children + 1));
		if(tmp != NULL){
			parent->children = tmp;
			parent->children[parent->num_children] = child;
			parent->num_children++;
		}
	}
}

void UIElementAddClass(UIElement *element, UIClass *class){
	if((element != NULL) && (class != NULL)){
		element->num_classes++;
		UIClass **tmp = realloc(element->classes, sizeof(UIClass *) * (element->num_classes + 1));
		if(tmp != NULL){
			element->classes = tmp;

			element->classes[element->num_classes - 1] = class;
		}else{
			DebugLog(D_ERR, "%s:%s: error: Could not allocate space in element's class buffer", __FILE__, __LINE__);
		}
	}
}

void UIElementAddTmpClass(UIElement *element, UIClass *class){
	if((element != NULL) && (class != NULL)){
		element->num_tmp_classes++;
		UIClass **tmp = realloc(element->tmp_classes, sizeof(UIClass *) * (element->num_tmp_classes + 1));
		if(tmp != NULL){
			element->tmp_classes = tmp;

			element->tmp_classes[element->num_tmp_classes - 1] = class;
		}else{
			DebugLog(D_ERR, "%s:%s: error: Could not allocate space in element's tmp class buffer", __FILE__, __LINE__);
		}
	}
}

UIClass *UIFindClass(UIState *state, char *name){
    UIClass *c = NULL;
	if((state != NULL) && (name != NULL)){
		for(int i = 0; i < state->num_classes; i++){
			if((state->classes[i].name != NULL) && (strcmp(name, state->classes[i].name) == 0)){
				c = &state->classes[i];
				break;
			}
		}
	}
	return c;
}

UIElement *UIFindElement(UIState *state, char *name){
    UIElement *e = NULL;
	if((state != NULL) && (name != NULL)){
		for(int i = 0; i < state->num_elements; i++){
			if((state->elements[i].name != NULL) && (strcmp(name, state->elements[i].name) == 0)){
				e = &state->elements[i];
				break;
			}
		}
	}

	if(e == NULL){
		DebugLog(D_WARN, "warning: %s: cannot find element with name '%s'", state->path, name);
	}

	return e;
}

void UIUpdate(UIState *state);
void UIInteract(UIState *state);
void UIRender(UIState *state);
void UIPush(UIState *state){
	// Determine element sizing and positioning
	UIUpdate(state);

	// Evaluate element-mouse interaction
    UIInteract(state);

	// Call rendering API and render UI elements to screen
    UIRender(state);
}