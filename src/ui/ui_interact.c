#include "../global.h"
#include "../debug.h"
#include "../event.h"

#include "ui.h"
#include "ui_layout.h"
#include "ui_interact.h"

static bool PointInBounds(Vector2_i point, Vector4 bounds){
    return ( (point.x >= bounds.x) && (point.x < (bounds.x + bounds.z)) &&
             (point.y >= bounds.y) && (point.y < (bounds.y + bounds.w)) );
}

static void ApplyAction(UIElement *element, UIClass *class, UI_Action action){
	if(class->actions[action].enabled){
		if(class->actions[action].function != NULL){
			class->actions[action].function();
		}
		for(int i = 0; i < class->actions[action].num_classes; i++){
			if(&class->actions[action].classes[i] != &class){
				ApplyClass(element, class->actions[action].classes[i]);
				if(action == UI_ACT_HOVER){
					// printf("hovering: %s, class: %s\n", element->name, class->name);

				}
			}
		}
		element->scene->needs_update = true;
	}
}

void CheckInteractions(UIElement *element){
	if(element->is_active){
		for(int i = 0; i < element->num_classes; i++){
			// Check if mouse is currently over the element
			if(PointInBounds(mouse_pos, element->transform)){ // ONHOVER
				ApplyAction(element, element->classes[i], UI_ACT_HOVER);
				// printf("HOVER\n");

				if(!PointInBounds(mouse_pos_previous, element->transform)){ // ONENTER
					ApplyAction(element, element->classes[i], UI_ACT_ENTER);

				}
				if(mouse_held){ // ONHOLD
					ApplyAction(element, element->classes[i], UI_ACT_HOLD);

				}
				if(mouse_clicked){ // ONCLICK
					ApplyAction(element, element->classes[i], UI_ACT_CLICK);
					element->is_selected = true;

				}
			}else{
				// Check if mouse was over the element in the previous frame
				if(PointInBounds(mouse_pos_previous, element->transform)){ // ONLEAVE
					ApplyAction(element, element->classes[i], UI_ACT_LEAVE);

				}
			}
			if(mouse_lifted){
				// If the element was previously selected and the mouse was released, deselect the element
				if(element->is_selected){ // ONRELEASE
					ApplyAction(element, element->classes[i], UI_ACT_RELEASE);
					element->is_selected = false;

				}
			}
		}
	}
}