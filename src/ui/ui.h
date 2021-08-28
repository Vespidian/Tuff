#ifndef UI_H_
#define UI_H_

extern TilesheetObject ui_tilesheet;

extern ShaderObject ui_shader;

void InitUI();

#include "../renderer/render_text.h"

typedef enum UI_Action_et{UI_ACT_HOVER, UI_ACT_CLICK, UI_ACT_ENTER, UI_ACT_LEAVE, UI_ACT_HOLD}UI_Action_et;
typedef enum UI_EaseType_et{UI_EASE_LINEAR}UI_EaseType_et; // https://www.easings.net
typedef enum UI_StyleType_et{
	UI_PIXELS,						// Specifies length in pixels
	UI_PERCENT,						// Specified length as a percentage of parent element
	UI_TRANSFORM_ABSOLUTE,			// Position in pixels relative to the body of the scene (does not apply to scale)
	UI_TRANSFORM_PIXELS_INVERTED,	// Position in pixels relative to the opposite side of the parent (does not apply to scale)
									// i.e. if the x position is 5px and 'UI_TRANSFORM_INVERTED_POSITION' is applied
									// the actual x position will be 5 pixels from the right edge of the parent element
	UI_STYLE_INHERIT				// Specified length equal to the parent element's
}UI_StyleType_et;

typedef enum UI_OriginType_et{
	UI_ORIGIN_TOP_LEFT = 0,
	UI_ORIGIN_TOP_MIDDLE,
	UI_ORIGIN_TOP_RIGHT,
	UI_ORIGIN_CENTER,
	UI_ORIGIN_CENTER_LEFT,
	UI_ORIGIN_CENTER_RIGHT,
	UI_ORIGIN_BOTTOM_LEFT,
	UI_ORIGIN_BOTTOM_MIDDLE,
	UI_ORIGIN_BOTTOM_RIGHT,
}UI_OriginType_et;

typedef enum UI_Align_et{
	UI_ALIGN_HORIZONTAL,
	UI_ALIGN_VERTICAL
}UI_Align_et;

typedef struct UIClass{
	char *name;

	Vector4_i transform_type;
	Vector4 transform; // Transforms the element relative to its parent's position
	Vector4 transform_relative; // Transforms the element relative to its current position

	Vector4 color;
	Vector4 border_color;
	Vector4 text_color;

	FontObject *font;
	float text_size;

	Vector4_i margin_type;
	Vector4 margin;

	Vector4_i border_type;
	Vector4 border;

	Vector4_i padding_type;
	Vector4 padding;

	Vector4_i radius_type;
	Vector4 radius;


	unsigned int transition_length; // Defines the length of property transitions in ms, 0 = instant
	UI_EaseType_et ease;

	struct UIAction *actions;
}UIClass;

typedef struct UIAction{
	UI_Action_et type;
	union{
		void (*hover)(void);
		void (*click)(void);
		void (*enter)(void);
		void (*leave)(void);
		void (*hold)(void);
	};
	UIClass *classes; // If this action is called from a class, that class cannot also be nested in the action
}UIAction;

typedef struct UIElement{
	char *name;

	struct UIElement *parent;
	unsigned int num_children;
	struct UIElement *children;

	TextureObject *image;

	Vector4 color;
	Vector4 border_color;
	Vector4 text_color;

	char *text;
	FontObject *font;
	float text_size;

	bool origin_set;
	UI_OriginType_et origin;

	Vector4_i transform_type;
	Vector4 transform; 			// Transforms the element relative to the parent's position
	Vector4 transform_absolute; // Calculated value inherited from all parent elements (or simply the transform if absolute positioning is used)
	Vector4 transform_calculated;

	UI_Align_et align;

	// For the below style properties the format is:
	// up, right, down, left
	Vector4_i margin_type;
	Vector4 margin;
	Vector4 margin_calculated;

	Vector4_i border_type;
	Vector4 border;
	Vector4 border_calculated;

	Vector4_i padding_type;
	Vector4 padding;
	Vector4 padding_calculated;

	// Format / order:
	// top_left, top_right, bottom_right, bottom_left
	Vector4_i radius_type;
	Vector4 radius;


	float ease_position;
	UIClass *classes; // Array of all classes applied to this element

	// UIAction actions[NUM_UI_ACTIONS];
	UIAction *actions; // Array of actions that this element triggers

	bool is_active; // Element visibility

}UIElement;

typedef struct UIScene{
	bool dynamic; // Determines whether or not 'needs_update' is used, if the seen is dynamic we re-compute it every frame no matter what
	bool needs_update; // Polled every frame to determine whether to re-compute the scene

	unsigned int num_classes; // Number of classes in the current scene
	UIClass *classes; // Array of all classes in the current scene
	UIElement body; // Root element (similar to 'body' in html)
}UIScene;


UIScene *uiLoadFile(UIScene *scene);
void RenderUIInstance(UIScene *scene);


#endif