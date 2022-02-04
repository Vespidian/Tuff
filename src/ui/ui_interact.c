#include "../global.h"
#include "../debug.h"
#include "../event.h"

#include "ui.h"
#include "ui_layout.h"
#include "ui_interact.h"

static bool PointInBounds(iVector2 point, Vector4 bounds){
    return ( (point.x >= bounds.x) && (point.x < (bounds.x + bounds.z)) &&
             (point.y >= bounds.y) && (point.y < (bounds.y + bounds.w)) );
}

static void ApplyAction(UIElement *element, UI_Action action){
	UIClass *class;
	for(int i = 0; i < element->num_classes; i++){
		class = element->classes[i];
		if(class->actions[action].enabled){
			// if(class->actions[action].function != NULL){
			// 	class->actions[action].function();
			// }
				// printf("Element name: %s\n", element->name);
			for(int j = 0; j < class->actions[action].num_classes; j++){
				if(&class->actions[action].classes[j] != &class){
					// printf("class name:, Address: %p\n", &class->actions[action].classes[j]);
					// DebugLog(D_ACT, "%s", class->actions[action].classes[j]->name);
					// if(class->actions[action].classes[j]->name == NULL){
					// 	printf("woop woopw\n");
					// }
					ApplyClass(element, class->actions[action].classes[j]);
				}
			}
			element->domain->needs_update = true;
		}
	}
}

void CheckInteractions(UIElement *element){
	/** TODO: 
	 *  - Add octree / quadtree system for ui domain
	 *  - Check mouse collision against all elements in current quadtree
	 *  - Only apply actions to the element with the largest z value (the top-most element)
	 */
	if(element->is_active){
		// Check if mouse is currently over the element
		if(PointInBounds(mouse_pos, element->transform)){ // ONHOVER
			ApplyAction(element, UI_ACT_HOVER);

			if(!PointInBounds(mouse_pos_previous, element->transform)){ // ONENTER
				ApplyAction(element, UI_ACT_ENTER);

			}
			if(mouse_held){ // ONHOLD
				ApplyAction(element, UI_ACT_HOLD);

			}
			if(mouse_clicked){ // ONCLICK
				ApplyAction(element, UI_ACT_CLICK);
				element->is_selected = true;
				// if(element->function != NULL){
				// 	element->function(element);
				// }

			}
		}else{
			// Check if mouse was over the element in the previous frame
			if(PointInBounds(mouse_pos_previous, element->transform)){ // ONLEAVE
				ApplyAction(element, UI_ACT_LEAVE);

			}
		}
		if(mouse_lifted){
			// If the element was previously selected and the mouse was released, deselect the element
			if(element->is_selected){ // ONRELEASE
				ApplyAction(element, UI_ACT_RELEASE);
				element->is_selected = false;

			}
		}
	}
}

void RecursiveCheckInteract(UIElement *element){
	if(element != NULL){
		if(element->is_active){
			for(int i = 0; i < element->num_children; i++){
				RecursiveCheckInteract(&element->children[i]);
			}
			CheckInteractions(element);
		}
	}
}