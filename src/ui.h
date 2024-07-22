#ifndef UI_LAYOUT_H_
#define UI_LAYOUT_H_

#include "textures.h"

typedef enum UI_ORIGIN {
	UI_ORIGIN_UNDEFINED = -1,
	UI_ORIGIN_NORTHWEST,
	UI_ORIGIN_NORTH,
	UI_ORIGIN_NORTHEAST,
	UI_ORIGIN_WEST,
	UI_ORIGIN_CENTER,
	UI_ORIGIN_EAST,
	UI_ORIGIN_SOUTHWEST,
	UI_ORIGIN_SOUTH,
	UI_ORIGIN_SOUTHEAST
}UI_ORIGIN;

typedef enum UI_MOUSE_EVENT{
	// Triggered once
	UI_MOUSE_CLICK   = 1 << 0,
	UI_MOUSE_RELEASE = 1 << 1,
	UI_MOUSE_ENTER   = 1 << 2,
	UI_MOUSE_LEAVE   = 1 << 3,

	// Triggered every frame
	UI_MOUSE_HOLD    = 1 << 4,
	UI_MOUSE_HOVER   = 1 << 5,
}UI_MOUSE_EVENT;

typedef enum UI_INPUT_TYPE{
	UI_INPUT_NONE,
	UI_INPUT_BUTTON,
	UI_INPUT_SLIDER,
	UI_INPUT_CHECKBOX,

}UI_INPUT_TYPE;

/**
 * IGNORES: (When elements of a class have these values, 
 * they are ignored and not passed to the element)
 * (Only the subelement with the specified value is ignored)
 * 
 * iVector2 offset					any xy = 2147483647
 * iVector2 size_min				any xy = -1
 * iVector2 size_max				any xy = -1
 * iVector4 padding					any xyzw = -1
 * iVector4 border					any xyzw = -1
 * iVector4 margin					any xyzw = -1
 * Vector3 color					any xyz = -1
 * int8_t wrap						-1
 * int8_t wrap_vertical				-1
 * int8_t wrap_reverse				-1
 * UI_ORIGIN origin_p				UI_ORIGIN_UNDEFINED
 * UI_ORIGIN origin_c				UI_ORIGIN_UNDEFINED
 * int8_t culling					-1
 * bool  inherit					N/A (not passed to element)
 * 
 * 
 * DEFAULTS: (The element.style class's default values)
 * 
 * iVector2 offset					(0, 0)
 * iVector2 size_min				100					(TBD)
 * iVector2 size_max				-1					(TBD)
 * iVector4 padding					10
 * iVector4 border					0
 * iVector4 margin					10
 * Vector3 color					(1, 1, 1)
 * int8_t wrap						true
 * int8_t wrap_vertical				false
 * int8_t wrap_reverse				false
 * UI_ORIGIN origin_p				UI_ORIGIN_NORTHWEST
 * UI_ORIGIN origin_c				UI_ORIGIN_NORTHWEST
 * bool  inherit					false
 * 
*/

extern bool ui_focused;

/**
 * Future refactoring idea:
 * Each class is an array of effects that can be applied to an element.
 * Each element of this array contains a union of possible value datatypes
 * and an enum value which dictates what type of effect it is.
 * 
 * This should make for a more efficient (faster) and much more expandable
 * system of adding different class effects
*/
typedef struct UIElement UIElement;
typedef struct UIState UIState;

typedef void (*UIMouseEventFunc)(UIElement *element);
typedef void (*UIMouseEventFunc_c)(UIState *state, UIElement *element, UI_MOUSE_EVENT events);

typedef struct UISlider{
	float val;
	float min;
	float max;
	float step;
	UIMouseEventFunc_c func;

	bool modify_width;
}UISlider;

typedef struct UIToggle{
	bool state;
	UIMouseEventFunc_c func;
	struct UIClass *c1;
	struct UIClass *c2;
}UIToggle;

typedef struct UIClass{
	int id;
	char *name;

	iVector2 offset;

	// Smallest size the element could have (no children)
	iVector2 size_min;

	// Biggest size the element could have (children fully stretch it)
	iVector2 size_max;

	// size_min as a percentage of the parent's size
	Vector2 size_min_percent;

	// size_max as a percentage of the parent's size
	Vector2 size_max_percent;

	// padding (left, top, right, bottom) -> (x, y, z, w)
	iVector4 padding;

	// border width (left, top, right, bottom) -> (x, y, z, w)
	iVector4 border;

	// margin (left, top, right, bottom) -> (x, y, z, w)
	iVector4 margin;

	// color
	Vector3 color;

	// border color
	Vector3 border_color;

	// Defines whether or not a child will be placed in the next 
	// row or column when there is no more room in the current
	int8_t wrap;

	// Defines whether to add new children to the next column (false) 
	// or next row (true)
	int8_t wrap_vertical;

	// Defines whether to add children in a forwards (left to right 
	// and top to bottom) or reverse order
	int8_t wrap_reverse;

	// The point of this element to align with the parent's child origin
	UI_ORIGIN origin_c;

	// Where on this element a child should align its parent origin
	UI_ORIGIN origin_p;

	// Whether or not children can be seen when outside the bounds 
	// of this parent element
	int8_t culling;

	// When 'inherit' is true, the children of this element will be 
	// automatically given this class
	bool inherit;

	/** MOUSE EVENTS **/
	char *class_hold;
	char *class_hover;

	UIMouseEventFunc_c event_func;

}UIClass;


typedef struct UIElement{
	int id;
	
	char *name;
	char *text;

	struct UIElement *parent;
	struct UIElement **children;
	unsigned int num_children;

	UIClass **classes;
	unsigned int num_classes;

	// Array of classes which gets emptied when they are applied 
	// to the elmeent
	UIClass **tmp_classes;
	unsigned int num_tmp_classes;

	Texture texture;

	// Final absolute positioning and scale
	iVector4 transform;
	iVector4 offset;

	UI_MOUSE_EVENT mouse_events;

	bool visible;
	bool visible_children;

	UI_INPUT_TYPE input_type;
	struct UISlider slider;


	// The final class to be applied to the element
	UIClass class;

	// The conglomerate of all classes in the class array 
	// (used for layout and rendering) (cannot be inherited)
	// 
	// This is a temporary class which is created from inherited 
	// parent classes, and classes in the classes array
	UIClass style;

	UIMouseEventFunc_c event_func;



}UIElement;

#define UI_STATE_MAX_CLASSES 1024
#define UI_STATE_MAX_ELEMENTS 1024
typedef struct UIState{
	// Element 0 is the root element that is automatically set 
	// when initializing a UIState
	char *path;
	
	// Array of all elements loaded into this UIState
	UIElement elements[UI_STATE_MAX_ELEMENTS];
	unsigned int num_elements;
	
	// Array of all classes loaded into this UIState
	UIClass classes[UI_STATE_MAX_CLASSES];
	unsigned int num_classes;

	// UIElement the mouse is currently hovered over (NULL means mouse is over elements[0] or 'root')
	UIElement *focused_element;


}UIState;




/**
 * --- LAYOUT ---
*/
extern unsigned int UI_WINDOW_WIDTH;
extern unsigned int UI_WINDOW_HEIGHT;

/**
 * @brief Initialize an empty UIState
*/
UIState UINewState();

/**
 * @brief Opens 'path' and attempts to load it into 'state'
 * @param state Pointer to state which will contain data loaded from file
 * @param path File to load as UI data
*/
void UIParse(UIState *state, char *path);

/**
 * @brief Frees 'state' and any internally allocated data
 * @param state Pointer to UIState to be free'd
*/
void UIFreeState(UIState *state);


void UIElementAddClass(UIElement *element, UIClass *class);
void UIElementAddTmpClass(UIElement *element, UIClass *class);

/**
 * @brief Find an element in 'state' called 'name' and return a pointer to it
 * @param state State in which to search for the element
 * @param name Name of the element to search for
 * @return Pointer to the found element - NULL means 'name' does not exist in 'state'
*/
UIElement *UIFindElement(UIState *state, char *name);

/**
 * @brief Find a class in 'state' called 'name' and return a pointer to it
 * @param state State in which to search for the class
 * @param name Name of the class to search for
 * @return Pointer to the found class - NULL means 'name' does not exist in 'state'
*/
UIClass *UIFindClass(UIState *state, char *name);


/**
 * --- INTERACT ---
*/

/**
 * @brief Set the function to be called when a mouse event happens to an element with 'class' applied to it
 * @param class Class which 'event_func' will be applied through
 * @param event_func Function to be called when a mouse event happens to an element
*/
void UIClassSetEventFunc(UIClass *class, UIMouseEventFunc_c event_func);

/**
 * @brief Set the hold class of another class
 * @param class Class which 'event_func' will be applied through
 * @param event_func 'event_class' gets applied to an element which has 'class' applied when the mouse is clicked and held on said element
*/
void UIClassSetEventClass_hold(UIClass *class, UIClass *event_class);

/**
 * @brief Set the hover class of another class
 * @param class Class which 'event_func' will be applied through
 * @param event_func 'event_class' gets applied to an element which has 'class' applied when the mouse is hovered over said element
*/
void UIClassSetEventClass_hover(UIClass *class, UIClass *event_class);


/**
 * @brief Turn 'element' into a slider object which can be interacted with
 * @param element Element to become a slider
 * @param min Minimum value
 * @param max Maximum value
 * @param val_default Starting value
 * @param step Change of slider value equivalent to the mouse moving 1 pixel
*/
void UISliderNew(UIElement *element, float min, float max, float val_default, float step);


/**
 * --- RENDER ---
*/

/**
 * @brief Initialize the UI rendering subsystem
*/
void InitUIRender();

/**
 * @brief Evaluates size and positioning of elements, and renders output to the screen
 * @param state Pointer to the UIState to be evaluated
*/
void UIPush(UIState *state);

#endif