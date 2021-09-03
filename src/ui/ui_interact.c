#include "../global.h"
#include "../debug.h"
#include "../event.h"

#include "ui_layout.h"
#include "ui_interact.h"

static bool PointInBounds(Vector2_i point, Vector4 bounds){
    return ( (point.x >= bounds.x) && (point.x < (bounds.x + bounds.z)) &&
             (point.y >= bounds.y) && (point.y < (bounds.y + bounds.w)) );
}

static void ApplyAction(UIElement *element, UIClass *class, UI_Action action){
	if(class->actions[action].function != NULL){
		class->actions[action].function();
	}
	for(int i = 0; i < class->actions[action].num_classes; i++){
		if(&class->actions[action].classes[i] != class){
			ApplyClass(element, &class->actions[action].classes[i]);
		}
	}
}

void CheckInteractions(UIElement *element){
	for(int i = 0; i < element->num_classes; i++){
		// Check if mouse is currently over the element
		if(PointInBounds(mouse_pos, element->transform)){ // ONHOVER
		// printf("HOVER\n");
			ApplyAction(element, element->classes[i], UI_ACT_HOVER);

			if(!PointInBounds(mouse_pos_previous, element->transform)){ // ONENTER
				ApplyAction(element, element->classes[i], UI_ACT_ENTER);

			}
			if(mouse_held){ // ONHOLD
				ApplyAction(element, element->classes[i], UI_ACT_HOLD);

			}
			if(mouse_clicked){ // ONCLICK
				ApplyAction(element, element->classes[i], UI_ACT_CLICK);

			}
		}else{
			// Check if mouse was over the element in the previous frame
			if(PointInBounds(mouse_pos_previous, element->transform)){ // ONLEAVE
				ApplyAction(element, element->classes[i], UI_ACT_LEAVE);

			}
		}
	}
}