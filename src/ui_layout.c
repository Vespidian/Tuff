#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "vectorlib.h"
#include "debug.h"
#include "ui.h"

unsigned int UI_WINDOW_WIDTH = 100;
unsigned int UI_WINDOW_HEIGHT = 100;

extern UIClass UIDefaultElementClass();

void UIElementApplyClass(UIElement *element, UIClass *class){
	if((element != NULL) && (class != NULL)){
		// Individually apply each bit of the class if it has been defined

		(class->offset.x == 2147483647) ? true : (element->style.offset.x = class->offset.x);
		(class->offset.y == 2147483647) ? true : (element->style.offset.y = class->offset.y);

		(class->size_min.x == -1) ? true : (element->style.size_min.x = class->size_min.x);
		(class->size_min.y == -1) ? true : (element->style.size_min.y = class->size_min.y);
		
		(class->size_max.x == -1) ? true : (element->style.size_max.x = class->size_max.x);
		(class->size_max.y == -1) ? true : (element->style.size_max.y = class->size_max.y);

		(class->size_min_percent.x == -1) ? true : (element->style.size_min_percent.x = class->size_min_percent.x);
		(class->size_min_percent.y == -1) ? true : (element->style.size_min_percent.y = class->size_min_percent.y);
		
		(class->size_max_percent.x == -1) ? true : (element->style.size_max_percent.x = class->size_max_percent.x);
		(class->size_max_percent.y == -1) ? true : (element->style.size_max_percent.y = class->size_max_percent.y);

		// Padding
		(class->padding.x == -1) ? true : (element->style.padding.x = class->padding.x);
		(class->padding.y == -1) ? true : (element->style.padding.y = class->padding.y);
		(class->padding.z == -1) ? true : (element->style.padding.z = class->padding.z);
		(class->padding.w == -1) ? true : (element->style.padding.w = class->padding.w);

		// Border
		(class->border.x == -1) ? true : (element->style.border.x = class->border.x);
		(class->border.y == -1) ? true : (element->style.border.y = class->border.y);
		(class->border.z == -1) ? true : (element->style.border.z = class->border.z);
		(class->border.w == -1) ? true : (element->style.border.w = class->border.w);

		// Margin
		(class->margin.x == -1) ? true : (element->style.margin.x = class->margin.x);
		(class->margin.y == -1) ? true : (element->style.margin.y = class->margin.y);
		(class->margin.z == -1) ? true : (element->style.margin.z = class->margin.z);
		(class->margin.w == -1) ? true : (element->style.margin.w = class->margin.w);

		// Color
		(class->color.x == -1) ? true : (element->style.color.x = class->color.x);
		(class->color.y == -1) ? true : (element->style.color.y = class->color.y);
		(class->color.z == -1) ? true : (element->style.color.z = class->color.z);

		// Border Color
		(class->border_color.x == -1) ? true : (element->style.border_color.x = class->border_color.x);
		(class->border_color.y == -1) ? true : (element->style.border_color.y = class->border_color.y);
		(class->border_color.z == -1) ? true : (element->style.border_color.z = class->border_color.z);
		
		// Wrap
		(class->wrap == -1) ? true : (element->style.wrap = class->wrap);
		(class->wrap_vertical == -1) ? true : (element->style.wrap_vertical = class->wrap_vertical);
		(class->wrap_reverse == -1) ? true : (element->style.wrap_reverse = class->wrap_reverse);

		// Origins
		(class->origin_c == UI_ORIGIN_UNDEFINED) ? true : (element->style.origin_c = class->origin_c);
		(class->origin_p == UI_ORIGIN_UNDEFINED) ? true : (element->style.origin_p = class->origin_p);
	}
}

iVector2 UIElementCalculateOriginP(UIElement *element, UI_ORIGIN origin_type){
	iVector2 origin = {0, 0};

	if(element != NULL){
		iVector2 size;

		origin = (iVector2){
			element->transform.x + element->style.border.x + element->style.padding.x, 
			element->transform.y + element->style.border.y + element->style.padding.y
		};

		size.x = 
			element->transform.z - 
			element->style.border.x - 
			element->style.border.z - 
			element->style.padding.x - 
			element->style.padding.z
		;

		size.y = 
			element->transform.w - 
			element->style.border.y - 
			element->style.border.w - 
			element->style.padding.y - 
			element->style.padding.w
		;

		switch(origin_type){
			case UI_ORIGIN_UNDEFINED:
			case UI_ORIGIN_NORTHWEST:
				// This is the default
				break;

			case UI_ORIGIN_NORTH:
				origin.x += size.x / 2;
				break;

			case UI_ORIGIN_NORTHEAST:
				origin.x += size.x;
				break;

			case UI_ORIGIN_WEST:
				origin.y += size.y / 2;
				break;

			case UI_ORIGIN_CENTER:
				origin.x += size.x / 2;
				origin.y += size.y / 2;
				break;

			case UI_ORIGIN_EAST:
				origin.x += size.x;
				origin.y += size.y / 2;
				break;

			case UI_ORIGIN_SOUTHWEST:
				origin.y += size.y;
				break;

			case UI_ORIGIN_SOUTH:
				origin.x += size.x / 2;
				origin.y += size.y;
				break;

			case UI_ORIGIN_SOUTHEAST:
				origin.x += size.x;
				origin.y += size.y;
				break;
			
		}
	}

	return origin; 
}


iVector2 UIElementCalculateOriginC(UIElement *element){
	iVector2 origin = {0, 0};
	if(element != NULL){
		iVector2 size;

		size.x = element->transform.z;
		size.y = element->transform.w;

		switch(element->style.origin_c){
			case UI_ORIGIN_UNDEFINED:
			case UI_ORIGIN_NORTHWEST:
				// This is the default
				
				origin.x += element->style.margin.x;
				origin.y += element->style.margin.y;
				break;

			case UI_ORIGIN_NORTH:
				origin.x -= size.x / 2;
				origin.y += element->style.margin.y;
				break;

			case UI_ORIGIN_NORTHEAST:
				origin.x -= size.x + element->style.margin.z;
				origin.y += element->style.margin.y;
				break;

			case UI_ORIGIN_WEST:
				origin.x += element->style.margin.x;
				origin.y -= size.y / 2;
				break;

			case UI_ORIGIN_CENTER:
				origin.x -= size.x / 2;
				origin.y -= size.y / 2;
				break;

			case UI_ORIGIN_EAST:
				origin.x -= size.x + element->style.margin.z;
				origin.y -= size.y / 2;
				break;

			case UI_ORIGIN_SOUTHWEST:
				origin.x += element->style.margin.x;
				origin.y -= size.y + element->style.margin.w;
				break;

			case UI_ORIGIN_SOUTH:
				origin.x -= size.x / 2;
				origin.y -= size.y + element->style.margin.w;
				break;

			case UI_ORIGIN_SOUTHEAST:
				origin.x -= size.x + element->style.margin.z;
				origin.y -= size.y + element->style.margin.w;
				break;
		}
	}

	return origin;
}


// Must only be called after figuring out the element's size
void UIElementUpdatePosition(UIElement *element){
	if(element != NULL){

		iVector2 offset;
		if(element->parent == NULL){
			offset = (iVector2){element->transform.x, element->transform.y};
		}else{
			offset = UIElementCalculateOriginP(element->parent, element->style.origin_p);

			// When there are existing siblings, we must find the last sibling
			// and position relative to that
			int i = -1;
			for(int k = 0; (element->parent->children[k] != element); k++){
				if(element->parent->children[k]->style.origin_p == element->style.origin_p){
					i = k;
				}
			}
			// I'th element is now the element right before 'element'
			if(i != -1){
				if(element->parent->style.wrap_vertical == false){
					// HORIZONTAL

					if(element->parent->style.wrap_reverse){
						// REVERSE

						offset.x = 
							element->parent->children[i]->transform.x - 
							element->parent->children[i]->style.margin.x - 
							element->style.margin.z - 
							element->transform.z;
					}else{
						// FORWARD

						offset.x = 
							element->parent->children[i]->transform.x + 
							element->parent->children[i]->transform.z + 
							element->parent->children[i]->style.margin.z + 
							element->style.margin.x;
					}
					offset.y += UIElementCalculateOriginC(element).y;
				}else{
					// VERTICAL

					if(element->parent->style.wrap_reverse){
						// REVERSE

						offset.y = 
							element->parent->children[i]->transform.y - 
							element->parent->children[i]->style.margin.y -
							element->style.margin.w - 
							element->transform.w;
					}else{
						// FORWARD

						offset.y = 
							element->parent->children[i]->transform.y + 
							element->parent->children[i]->transform.w + 
							element->parent->children[i]->style.margin.w + 
							element->style.margin.y;
					}

					offset.x += UIElementCalculateOriginC(element).x;
				}
			}else{
				iVector2 origin_offset = UIElementCalculateOriginC(element);
				offset.x += origin_offset.x;
				offset.y += origin_offset.y;
			}
		}
		
		element->transform.x = offset.x + element->offset.x;
		element->transform.y = offset.y + element->offset.y;
	}
}


void UIElementUpdateSize(UIElement *element){
	if(element != NULL){
		// Reset element's style class
		element->style = UIDefaultElementClass();

		// Condense all classes into a single 'element.style' class by 
		// calling 'UIElementApplyClass'

		// Look at parent for inherited classes
		if(element->parent != NULL){
			for(int i = 0; i < element->parent->num_classes; i++){
				if(element->parent->classes[i]->inherit){
					UIElementApplyClass(element, element->parent->classes[i]);
				}
			}
		}

		// Look at element's own classes for other modification
		for(int i = 0; i < element->num_classes; i++){
			UIElementApplyClass(element, element->classes[i]);
		}

		// Apply temporary classes and clear buffer
		for(int i = 0; i < element->num_tmp_classes; i++){
			UIElementApplyClass(element, element->tmp_classes[i]);
		}
		element->num_tmp_classes = 0;
		free(element->tmp_classes);
		element->tmp_classes = NULL;

		// And finally, apply the element's own class to itself
		UIElementApplyClass(element, &element->class);

		// Look at children for size and any stretching
		element->transform.z = 
			element->style.border.x + 
			element->style.border.z + 
			element->style.padding.x + 
			element->style.padding.z + 
			element->offset.z
		;
		element->transform.w = 
			element->style.border.y + 
			element->style.border.w + 
			element->style.padding.y + 
			element->style.padding.w + 
			element->offset.w
		;

		if(element->visible_children){
			if(element->style.wrap_vertical == true){
				// VERTICAL
				int widest = 0;
				for(int i = 0; i < element->num_children; i++){
					int sum = 0;
					
					// Child size
					element->transform.w += element->children[i]->transform.w;
					sum += element->children[i]->transform.z;

					// Child margin
					element->transform.w += element->children[i]->style.margin.y;
					element->transform.w += element->children[i]->style.margin.w;
					sum += element->children[i]->style.margin.x;
					sum += element->children[i]->style.margin.z;

					if(sum > widest){
						widest = sum;
					}

				}
				element->transform.z += widest;

			}else{
				// HORIZONTAL
				int tallest = 0;
				for(int i = 0; i < element->num_children; i++){
					if(element->children[i]->visible){
						int sum = 0;

						// Child size
						element->transform.z += element->children[i]->transform.z;
						sum += element->children[i]->transform.w;

						// Child margin
						element->transform.z += element->children[i]->style.margin.x;
						element->transform.z += element->children[i]->style.margin.z;
						sum += element->children[i]->style.margin.y;
						sum += element->children[i]->style.margin.w;

						if(sum > tallest){
							tallest = sum;
						}
					}

				}
				element->transform.w += tallest;
			}
		}

		if(element->parent != NULL){
			(element->style.size_min_percent.x != -1) ? (element->style.size_min.x = (element->parent->transform.z * element->style.size_min_percent.x / 100.0f)) : false;
			(element->style.size_min_percent.y != -1) ? (element->style.size_min.y = (element->parent->transform.w * element->style.size_min_percent.y / 100.0f)) : false;
			(element->style.size_max_percent.x != -1) ? (element->style.size_max.x = (element->parent->transform.z * element->style.size_max_percent.x / 100.0f)) : false;
			(element->style.size_max_percent.y != -1) ? (element->style.size_max.y = (element->parent->transform.w * element->style.size_max_percent.y / 100.0f)) : false;
		}

		// Limit size of element if children go beyond max size
		if((element->transform.z > element->style.size_max.x) && (element->style.size_max.x != -1)){
			element->transform.z = element->style.size_max.x;
		}else if(element->transform.z < element->style.size_min.x){
			element->transform.z = element->style.size_min.x;
		}

		if((element->transform.w > element->style.size_max.y) && (element->style.size_max.y != -1)){
			element->transform.w = element->style.size_max.y;
		}else if(element->transform.w < element->style.size_min.y){
			element->transform.w = element->style.size_min.y;
		}

		if(element->parent == NULL){
			element->transform.z = UI_WINDOW_WIDTH;
			element->transform.w = UI_WINDOW_HEIGHT;
		}

	}
}

// void UIElementUpdateChildren(UIElement *element){
void UIUpdate(UIState *state){
	if(state != NULL){
		// 1. iteratively creating an array of all nodes (elements)
		// 2. Iterate through the array backwards, updating each element based on 
		// the size of its immediate children and any classes inherited from 
		// the parent element

		// Initially we create an array of all children
		UIElement *element = &state->elements[0];
		UIElement **children;
		unsigned int num_children = 1;
		children = malloc(sizeof(UIElement) * (num_children + 1));
		children[0] = element;
		children[num_children + 1] = NULL;

		if(children[0]->num_children != 0){
			for(int i = 0; children[i] != NULL; i++){
				if(children[i]->num_children == 0){
					continue;
				}

				UIElement **tmp = realloc(children, sizeof(UIElement *) * (num_children + children[i]->num_children + 1));
				if(tmp != NULL){
					children = tmp;
					num_children += children[i]->num_children;

					for(int k = 0; k < children[i]->num_children; k++){
						children[num_children - children[i]->num_children + k] = children[i]->children[k];
					}
					children[num_children] = NULL;
				}

			}
		}


		// Loop from leaves to root, calculating the size of each element
		for(int i = (num_children - 1); i >= 0; i--){
			UIElementUpdateSize(children[i]);
		}

		for(int i = 0; i < num_children; i++){
			UIElementUpdatePosition(children[i]);
		}
			


		free(children);
		children = NULL;
	}
}

void UIElementUpdateSiblings(UIElement *element){
	// Update all siblings of 'element' as well as all siblings of all parents
	
	// Here, we start at a leaf of the tree. 
	// 1. Create a 'parent' variable, to store the parent of the current 
	// children we are looping through
	// 3. Loop through all immediate children of the 'parent' variable
	// 4. Update each child's size and classes based on its immediate children 
	// and any classes inherited from the parent element
	// 5. Set the 'parent' variable to its own parent
	// 6. Repeat steps 3 to 5 until the root parent

}

/**
 * Loop through tree once, creating array
 * Now, start at leaves and apply each element's classes, and 
 *  then determine size of each element (UIElementUpdateSize())
 * Then, start at root and determine position of every element
 *  (UIElementUpdatePosition())
*/
