#ifndef UI_H_
#define UI_H_

#include "../gl_utils.h"
#include "../renderer/render_text.h"

extern TilesheetObject ui_tilesheet;

extern ShaderObject ui_shader;

void InitUI();

#define UI_NUM_ACTIONS 6
typedef enum UI_Action{
	UI_ACT_HOVER = 0, 
	UI_ACT_ENTER, 
	UI_ACT_LEAVE, 
	UI_ACT_HOLD,
	UI_ACT_CLICK, 
	UI_ACT_RELEASE, 
}UI_Action;

typedef enum UI_EaseType{UI_EASE_UNDEFINED, UI_EASE_LINEAR}UI_EaseType; // https://www.easings.net
typedef enum UI_StyleType_et{
	UI_PIXELS,						// Specifies length in pixels
	UI_PERCENT,						// Specified length as a percentage of parent element
	UI_TRANSFORM_ABSOLUTE,			// Position in pixels relative to the body of the scene (does not apply to scale)
	UI_TRANSFORM_PIXELS_INVERTED,	// Position in pixels relative to the opposite side of the parent (does not apply to scale)
									// i.e. if the x position is 5px and 'UI_TRANSFORM_INVERTED_POSITION' is applied
									// the actual x position will be 5 pixels from the right edge of the parent element
	UI_STYLE_INHERIT,				// Specified length equal to the parent element's
	UI_UNDEFINED,					// Specifies value as default if a property is undefined
}UI_StyleType_et;

typedef union{
	struct{int x, y, z, w;};
	UI_StyleType_et v[4];
}UI_Property;

typedef enum UI_OriginType{
	UI_ORIGIN_TOP_LEFT,
	UI_ORIGIN_TOP_MIDDLE,
	UI_ORIGIN_TOP_RIGHT,
	UI_ORIGIN_CENTER,
	UI_ORIGIN_CENTER_LEFT,
	UI_ORIGIN_CENTER_RIGHT,
	UI_ORIGIN_BOTTOM_LEFT,
	UI_ORIGIN_BOTTOM_MIDDLE,
	UI_ORIGIN_BOTTOM_RIGHT,
}UI_OriginType;

typedef enum UI_Align{
	UI_ALIGN_HORIZONTAL,
	UI_ALIGN_VERTICAL
}UI_Align;
typedef struct UIClass UIClass;
typedef struct UIAction{
	void (*function)(void);
	UIClass **classes; // If this action is called from a class, that class cannot also be nested in the action
	unsigned int num_classes;
}UIAction;

typedef struct UIClass{
	char *name;

	bool font_defined;
	FontObject *font;

	bool text_size_defined;
	float text_size;
	

	bool color_defined;
	Vector3 color;
	bool opacity_defined;
	float opacity;


	bool border_color_defined;
	Vector3 border_color;
	bool border_opacity_defined;
	float border_opacity;

	bool text_color_defined;
	Vector3 text_color;
	bool text_opacity_defined;
	float text_opacity;

	UI_Align align;

	UI_OriginType origin;

	UI_Property transform_type;
	Vector4 transform; // Transforms the element relative to its parent's position

	UI_Property transform_relative_type;
	Vector4 transform_relative; // Transforms the element relative to its current position

	UI_Property margin_type;
	Vector4 margin;

	UI_Property border_type;
	Vector4 border;

	UI_Property padding_type;
	Vector4 padding;

	UI_Property radius_type;
	Vector4 radius;

	bool transition_defined;
	unsigned int transition_length; // Defines the length of property transitions in ms, 0 = instant
	UI_EaseType ease;

	UIAction actions[6];
}UIClass;

typedef struct UIElement{
	char *name;

	struct UIElement *parent;
	unsigned int num_children;
	struct UIElement *children;

	TextureObject *image;

	char *text;
	FontObject *font;
	float text_size;

	Vector4 color;
	Vector4 border_color;
	Vector4 text_color;

	// Values to be used for rendering (Calculated from classes or default values)
	Vector2 base_position;
	Vector2 base_scale;
	Vector4 transform;
	Vector4 margin;
	Vector4 border;
	Vector4 padding;
	Vector4 radius;

	bool full_screen;

	bool is_selected;

	float ease_position; // Value between 0 and 1 defining the current transition state
	UIClass **classes; // Array of all classes applied to this element
	unsigned int num_classes;

	// UIAction actions[NUM_UI_ACTIONS];
	// UIAction *actions; // Array of actions that this element triggers

	bool is_active; // Element visibility

}UIElement;

typedef struct UIScene{
	char *path;
	bool dynamic; // Determines whether or not 'needs_update' is used, if the seen is dynamic we re-compute it every frame no matter what
	bool needs_update; // Polled every frame to determine whether to re-compute the scene

	unsigned int num_classes; // Number of classes in the current scene
	UIClass *classes; // Array of all classes in the current scene
	UIElement body; // Root element (similar to 'body' in html)
}UIScene;

// TMP
extern UIScene *scene_stack;
UIScene *UI_LoadScene(char *path);
void UI_RenderScene(UIScene *scene);
void UI_FreeScene(UIScene *scene);


#endif