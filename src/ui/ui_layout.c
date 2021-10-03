#include "../global.h"
#include "../debug.h"

//UI Elements
#include "ui.h"
#include "ui_layout.h"

static Vector4 CalculatePadding(UIElement *element, UIClass *class){
	int axis = 0; // Axis (0 = x axis, 1 = y axis)
	Vector4 padding;
	for(int i = 0; i < 4; i++){
		axis = i % 2;
		bool type_defined = true;
		switch(class->padding_type.v[i]){
			case UI_PERCENT:
				padding.v[i] = element->parent->transform.v[axis + 2] * class->padding.v[i];
				break;
			case UI_STYLE_INHERIT:
				padding.v[i] = element->parent->padding.v[i];
				break;
			case UI_PIXELS:
				padding.v[i] = class->padding.v[i];
				break;
			default: // UI_UNDEFINED or anything not supported by this property
				type_defined = false;
				break;
		}
		if(type_defined){
			element->padding.v[i] = padding.v[i];
		}
	}
	return padding;
}

static Vector4 CalculateBorder(UIElement *element, UIClass *class){
	int axis = 0; // Axis (0 = x axis, 1 = y axis)
	Vector4 border;
	for(int i = 0; i < 4; i++){
		axis = i % 2;
		bool type_defined = true;
		switch(class->border_type.v[i]){
			case UI_PERCENT:
				border.v[i] = element->parent->transform.v[axis + 2] * class->border.v[i];
				break;
			case UI_STYLE_INHERIT:
				border.v[i] = element->parent->border.v[i];
				break;
			case UI_PIXELS:
				border.v[i] = class->border.v[i];
				break;
			default: // UI_UNDEFINED or anything not supported by this property
				type_defined = false;
				break;
		}
		if(type_defined){
			element->border.v[i] = border.v[i];
		}
	}
	return border;
}

static Vector2 CalculateScale(UIElement *element, UIClass *class){
	Vector2 scale = element->base_scale;
	Vector4 border = element->border;
	Vector4 padding = element->padding;
	for(int i = 2; i <= 3; i++){
		bool type_defined = true;
		element->scale_defined[i - 2] = true;
		switch(class->transform_type.v[i]){
			case UI_PERCENT:
				scale.v[i - 2] = element->parent->transform.v[i] * class->transform.v[i];
				break;
			case UI_PIXELS:
				scale.v[i - 2] = class->transform.v[i];
				break;
			default: // UI_UNDEFINED or anything not supported by this property
				type_defined = false;
				element->scale_defined[i - 2] = false;
				break;
		}
		if(type_defined){
			element->base_scale.v[i - 2] = scale.v[i - 2];
		}
		if(i == 2){
			scale.v[i - 2] += (border.v[i - 1] + border.v[i + 1]) + (padding.v[i - 1] + padding.v[i + 1]);
		}else{
			scale.v[i - 2] += (border.v[i - 1] + border.v[i - 3]) + (padding.v[i - 1] + padding.v[i - 3]);
		}
		element->transform.v[i] = scale.v[i - 2];
	}
	return scale;
}

static Vector4 CalculateMargin(UIElement *element, UIClass *class){
	int axis = 0; // Axis (0 = x axis, 1 = y axis)
	Vector4 margin;
	for(int i = 0; i < 4; i++){
		axis = i % 2;
		bool type_defined = true;
		switch(class->margin_type.v[i]){
			case UI_PERCENT:
				margin.v[i] = element->parent->transform.v[axis + 2] * class->margin.v[i];
				break;
			case UI_STYLE_INHERIT:
				margin.v[i] = element->parent->margin.v[i];
				break;
			case UI_PIXELS:
				margin.v[i] = class->margin.v[i];
				break;
			default: // UI_UNDEFINED or anything not supported by this property
				type_defined = false;
				break;
		}
		if(type_defined){
			element->margin.v[i] = margin.v[i];
		}
	}
	return margin;
}

static Vector2 CalculatePosition(UIElement *element, UIClass *class){
	Vector2 position = element->base_position;
	// Vector4 margin = element->margin;
	for(int i = 0; i <= 1; i++){
		bool type_defined = true;
		switch(class->transform_type.v[i]){
			case UI_PERCENT:
				position.v[i] = element->parent->transform.v[i + 2] * class->transform.v[i];
				break;
			case UI_TRANSFORM_PIXELS_INVERTED:
				position.v[i] = element->parent->transform.v[i + 2] - class->transform.v[i];
				break;
			case UI_TRANSFORM_ABSOLUTE:
				position.v[i] = class->transform.v[i];
				break;
			case UI_PIXELS:
				position.v[i] = class->transform.v[i];
				break;
			default: // UI_UNDEFINED or anything not supported by this property
				type_defined = false;
				break;
		}

		if(type_defined){
			element->base_position.v[i] = position.v[i];
			if(class->transform_type.v[i] == UI_TRANSFORM_PIXELS_INVERTED){ // bottom / right
				// Set up the current direction
				element->origin |= 1 << (i + 1);
				// Clear the opposite direction
				element->origin &= ~(1 << (!i * 3));
			}
		}
	}
	return position;
}

// This function only calculates offset for the internals of the element (border, padding, scale)
static Vector2 CalculateOffset(UIElement *element){
	Vector2 origin_offset = {0, 0};
	UI_OriginType origin = element->origin;
	Vector2 scale = (Vector2){
		element->base_scale.x + element->border.y + element->border.w + element->padding.y + element->padding.w,
		element->base_scale.y + element->border.x + element->border.z + element->padding.x + element->padding.z
	};
	
	// If the class has no definite origin, use the element's current origin
	if(origin == UI_ORIGIN_UNDEFINED){
		origin = element->origin;
	}

	// RETURN: stop offseting the position inside the position function, do it all here
	// once that is done, possibly combine all property setting functions into 1
	switch(origin){
		default: // UI_ORIGIN_TOP_LEFT or UI_UNDEFINED
			// By default the origin is in the top left corner, so no offset
			// origin_offset = (Vector2){-(element->parent->border.x + element->parent->padding.x), -(element->parent->border.y + element->parent->padding.y)};
			break;
		case UI_ORIGIN_TOP:
			origin_offset = (Vector2){scale.x / 2, 0};
			break;
		case UI_ORIGIN_TOP_RIGHT:
			origin_offset = (Vector2){scale.x, 0};
			break;

		case UI_ORIGIN_CENTER:
			origin_offset = (Vector2){scale.x / 2, scale.y / 2};
			break;
		case UI_ORIGIN_LEFT:
			origin_offset = (Vector2){0, scale.y / 2};
			break;
		case UI_ORIGIN_RIGHT:
			origin_offset = (Vector2){scale.x, scale.y / 2};
			break;

		case UI_ORIGIN_BOTTOM_LEFT:
			origin_offset = (Vector2){0, scale.y};
			break;
		case UI_ORIGIN_BOTTOM:
			origin_offset = (Vector2){scale.x / 2, scale.y};
			break;
		case UI_ORIGIN_BOTTOM_RIGHT:
			origin_offset = (Vector2){scale.x, scale.y};
			break;
	}

	element->origin_offset = (Vector2)origin_offset;
	element->transform.x = element->base_position.x;
	element->transform.y = element->base_position.y;
	return origin_offset;
}

static void CalculateRadiusLimit(float length, float *r1, float *r2){
	if(length - *r1 - *r2 < 0){
		float scale_factor = length / (*r1 + *r2);
		*r1 *= scale_factor;
		*r2 *= scale_factor;
	}
	if(*r1 * 2 > length){
		*r1 = length * 0.5f;
	}
	if(*r2 * 2 > length){
		*r2 = length * 0.5f;
	}
}

static void CalculateRadius(UIElement *element, UIClass *class){ // TODO: Add percent and inherit cases for radius
	for(int i = 0; i < 4; i++){
		switch(class->radius_type.v[i]){
			case UI_PIXELS:
				element->radius.v[i] = class->radius.v[i];
				break;
			default:
				break;
		}
	}
}

static void SetValues(UIElement *element, UIClass *class){
	if(class->color_defined){
		element->color = (Vector4){class->color.x, class->color.y, class->color.z, element->color.w};
	}
	if(class->opacity_defined){
		element->color.w = class->opacity;
	}
	if(class->border_color_defined){
		element->border_color = (Vector4){class->border_color.x, class->border_color.y, class->border_color.z, element->border_color.w};
	}
	if(class->border_opacity_defined){
		element->border_color.w = class->border_opacity;
	}
	if(class->text_color_defined){
		element->text_color = (Vector4){class->text_color.x, class->text_color.y, class->text_color.z, element->text_color.w};
	}
	if(class->text_opacity_defined){
		element->text_color.w = class->text_opacity;
	}


	if(class->text_size_defined){
		element->text_size = class->text_size;
	}
	if(class->font_defined){
		element->font = class->font;
	}
	// if(class->transition_defined){
		// element-> = class->;
	// }

	element->is_active = class->is_active;
}

// float Transition(int start, int current, int end, int time_period); // TODO: Implement transition animations / easings for classes

static void CalculateContentRect(UIElement *element){
	element->content_rect = (Vector4){
		// element->parent->transform.x + element->parent->border.x + element->parent->padding.x,
		// element->parent->transform.y + element->parent->border.y + element->parent->padding.y,
		element->transform.x + element->border.x + element->padding.x,
		element->transform.y + element->border.y + element->padding.y,
		element->transform.z,
		element->transform.w,
	};
}

static Vector2 CalculateFullBounds(UIElement *element){
	return (Vector2){
		element->margin.w + element->margin.y + element->border.w + element->border.y + element->padding.w + element->padding.y + element->base_scale.x,
		element->margin.x + element->margin.z + element->border.x + element->border.z + element->padding.x + element->padding.z + element->base_scale.y,
	};
}

static Vector2 CalculateChildPosition(UIElement *element, Vector2 offset){
	// TODO: add options for aligning elements to 'right' and 'bottom'
	// element->transform = (Vector4){
		element->transform.x += offset.x + element->margin.w - element->origin_offset.x;
		element->transform.y += offset.y + element->margin.x - element->origin_offset.y;
	// };

	Vector2 next_offset = CalculateFullBounds(element);
	if(element->parent->align == UI_ALIGN_HORIZONTAL){
		next_offset.x += offset.x;
		next_offset.y = offset.y;
	}else if(element->parent->align == UI_ALIGN_VERTICAL){
		next_offset.x = offset.x;
		next_offset.y += offset.y;
	}

	return next_offset;
}

void ApplyClass(UIElement *element, UIClass *class){
	if(class != NULL){
		CalculateBorder(element, class);
		CalculatePadding(element, class);
		CalculateScale(element, class);
		CalculateMargin(element, class);
		CalculatePosition(element, class);
		
		CalculateRadius(element, class);

		SetValues(element, class);


	}
}

static void LargestChild(Vector2 *current, UIElement *new_child){
	Vector2 child_coverage = CalculateFullBounds(new_child);
	if((child_coverage.x) > current->x){
		current->x = child_coverage.x;
	}
	if((child_coverage.y) > current->y){
		current->y = child_coverage.y;
	}
}

void RecursiveApplyStaticClasses(UIElement *element){
	ResetElement(element);
	for(int i = 0; i < element->num_classes; i++){
		ApplyClass(element, element->classes[i]);
	}
	if(element->children != NULL){
		for(int i = 0; i < element->num_children; i++){
			RecursiveApplyStaticClasses(&element->children[i]);
		}
	}
}

void RecursiveApplyElementClasses(UIElement *element){
	CalculateContentRect(element);
	CalculateOffset(element);

	if(element->children != NULL){
		for(int i = 0; i < element->num_children; i++){
			RecursiveApplyElementClasses(&element->children[i]);
		}
	}

	// Calculate positioning
	if(element->num_children != 0){
		// TODO: Fix origin offset with children
		Vector2 offset = (Vector2){element->base_position.x + element->padding.w + element->border.w, element->base_position.y + element->padding.x + element->border.x};
		// Now we calculate the size and positioning of elements relative to each other
		Vector2 largest_child = {0, 0};
		for(int i = 0; i < element->num_children; i++){
			offset = CalculateChildPosition(&element->children[i], offset);
			LargestChild(&largest_child, &element->children[i]);
		}

		largest_child.y *= (element->align == UI_ALIGN_HORIZONTAL);
		largest_child.x *= (element->align == UI_ALIGN_VERTICAL);

		if(!element->scale_defined[0]){
			element->transform.z = offset.x - element->transform.x + element->padding.y + element->border.y + largest_child.x;
		}
		if(!element->scale_defined[1]){
			element->transform.w = offset.y - element->transform.y + element->padding.z + element->border.z + largest_child.y;
		}

	}
	CalculateRadiusLimit(element->transform.z, &element->radius.z, &element->radius.w);
	CalculateRadiusLimit(element->transform.w, &element->radius.y, &element->radius.z);
	CalculateRadiusLimit(element->transform.w, &element->radius.w, &element->radius.x);
	CalculateRadiusLimit(element->transform.z, &element->radius.x, &element->radius.y);
}