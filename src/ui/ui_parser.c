#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <SDL.h>

#include "../global.h"
#include <jsmn.h>
#include "../debug.h"

#include "ui.h"
#include "ui_parser.h"

typedef struct JSONObject_t{
	char *json_string;
	jsmntok_t *tokens;
	int num_tokens;
}JSONObject_t;

typedef enum CurrentObject_e{
	UNKOWN_OBJECT,
	CLASS_BUFFER_OBJECT, 
	CLASS_OBJECT, 
	SCENE_OBJECT, 
	ELEMENT_OBJECT, 
	PROPERTY_OBJECT, 
	EVENT_OBJECT,
}CurrentObject_et;

static char *active_path;

unsigned int num_properties = 0;
const char *property_dict[] = {
	"position",
	"top",
	"bottom",
	"left",
	"right",

	"size",
	"width",
	"height",

	"rotation",

	"margin",
	"margin-top",
	"margin-right",
	"margin-bottom",
	"margin-left",

	"border",
	"border-top",
	"border-right",
	"border-bottom",
	"border-left",

	"padding",
	"padding-top",
	"padding-right",
	"padding-bottom",
	"padding-left",

	"radius",
	"radius-top-left",
	"radius-top-right",
	"radius-bottom-right",
	"radius-bottom-left",

	"opacity",
	"color",
	"border-opacity",
	"border-color",
	"text-opacity",
	"text-color",
	"text-size",
	"font",

	"align",

	"transition-duration",
	"transition-ease",

	"active",

	"onhover",
	"onenter",
	"onleave",
	"onhold",
	"onclick",
	"onrelease",

	NULL,

}; // MAKE SURE to keep a NULL at the end of this array for counting number of properties

void CountUIProperties(){
	while(property_dict[num_properties] != NULL){
		num_properties++;
	}
}

static int min(int a, int b){
	int result = a;
	if(b < a){
		result = b;
	}
	return result;
}

UIClass *UI_NewClass(UIDomain *domain){
	// TODO: Add a 'global domain' to allow for external classes that can be accessed from any .uiss file
	domain->classes = realloc(domain->classes, sizeof(UIClass) * ++domain->num_classes);
	UIClass *class = &domain->classes[domain->num_classes - 1];

	// class->name = malloc(1);
	class->name = 0;
	class->font_defined = false;
	class->font = &default_font;
	class->text_size_defined = false;
	class->color_defined = false;
	class->opacity_defined = false;
	class->border_color_defined = false;
	class->border_opacity_defined = false;
	class->text_color_defined = false;
	class->text_opacity_defined = false;
	class->transition_defined = false;
	class->is_active = true;
	class->align = UI_ALIGN_UNDEFINED;
	class->origin = UI_ORIGIN_UNDEFINED;

	class->transform_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->transform_relative_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->margin_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->border_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->padding_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->radius_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};

	class->ease = UI_EASE_UNDEFINED;

	for(int i = 0; i < UI_NUM_ACTIONS; i++){
		class->actions[i].enabled = false;
		class->actions[i].num_classes = 0;
		// class->actions[i].classes = malloc(sizeof(UIClass));
		class->actions[i].classes = NULL;
		// class->actions[i].function = NULL;
	}

	return class;
}


void ResetElement(UIElement *element){
	if(element != NULL){
		element->image = &undefined_texture;

		element->font = &default_font;
		element->text_size = 1;
		element->text_color = (Vector4){1, 1, 1, 1};

		element->color = (Vector4){1, 1, 1, 1};
		element->border_color = (Vector4){0, 0, 0, 1};

		element->base_position= (Vector2){0, 0};
		element->base_scale = (Vector2){0, 0};
		element->scale_defined[0] = false;
		element->scale_defined[1] = false;
		element->transform 	= (Vector4){0, 0, 50, 50};
		element->margin 	= (Vector4){0, 0, 0, 0};
		element->border 	= (Vector4){1, 1, 1, 1};
		element->padding 	= (Vector4){0, 0, 0, 0};
		element->radius 	= (Vector4){0, 0, 0, 0};

		element->align = UI_ALIGN_VERTICAL;
		element->origin = UI_ORIGIN_TOP_LEFT;

		// element->ease_position = 0;

		// element->is_selected = false;
		// element->is_active = true;
	}
}

UIElement *UI_NewElement(UIElement *parent){
	parent->children = realloc(parent->children, sizeof(UIElement) * ++parent->num_children);
	UIElement *element = &parent->children[parent->num_children - 1];

	element->name = 0;

	element->domain = parent->domain;

	element->parent = parent;
	element->num_children = 0;
	element->children = NULL;

	element->text = 0;

	element->num_classes = 0;
	element->classes = NULL;

	//tmp
	// element->function = NULL;

	ResetElement(element);


	return element;
}

void InitializeDomain(UIDomain *domain){
	domain->classes = NULL;
	domain->num_classes = 0;

	domain->dynamic = false; // Domains are static by default

	domain->needs_update = true;


	domain->body.name = malloc(sizeof(char) * 5);
	strcpy(domain->body.name, "body");
	domain->body.name[4] = 0;

	domain->body.domain = domain;

	domain->body.parent = NULL;
	domain->body.children = NULL;
	domain->body.num_children = 0;

	domain->body.image = false;

	domain->body.text = 0;
	domain->body.font = &default_font;
	domain->body.text_size = 1;
	domain->body.text_color = (Vector4){1, 1, 1, 1};

	domain->body.color = (Vector4){0, 0, 0, 0};
	domain->body.border_color = (Vector4){0, 0, 0, 1};

	domain->body.base_position= (Vector2){0, 0};
	domain->body.base_scale 	= (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT};
	domain->body.content_rect= (Vector4){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	domain->body.transform 	= (Vector4){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	domain->body.margin 		= (Vector4){5, 5, 5, 5};
	domain->body.border 		= (Vector4){0, 0, 0, 0};
	domain->body.padding 	= (Vector4){10, 10, 10, 10};
	domain->body.radius 		= (Vector4){0, 0, 0, 0};

	domain->body.align = UI_ALIGN_HORIZONTAL;
	domain->body.origin = UI_ORIGIN_TOP_LEFT;

	domain->body.full_screen = true;

	domain->body.ease_position = 0;

	domain->body.classes = NULL;
	domain->body.num_classes = 0;

	domain->body.is_selected = false;
	domain->body.is_active = true;
}

static int strntol(char *string, unsigned int length){
	int result = 0;
	int decimal_place = 1;
	for(int i = 0; i < length; i++){
		if(string[length - 1 - i] >= '0' && string[length - 1 - i] <= '9'){
			result += (string[length - 1 - i] - '0') * decimal_place;
			decimal_place *= 10;
		}
	}
	if(string[0] == '-'){
		result *= -1;
	}

	return result;
}

static Vector3 HexToColor(char *hex){
	if(hex[0] == '#'){
		hex = hex + 1;
	}
	short v[3] = {0};
	char o1, o2;
	o1 = o2 = 0;
	for(int i = 0; i < 3; i++){
		if(hex[i * 2] <= '9'){
			o1 = '0';
		}else if(hex[i * 2] <= 'F'){
			o1 = 'A' - 10;
		}else if(hex[i * 2] <= 'f'){
			o1 = 'a' - 10;
		}
		if(hex[i * 2 + 1] <= '9'){
			o2 = '0';
		}else if(hex[i * 2 + 1] <= 'F'){
			o2 = 'A' - 10;
		}else if(hex[i * 2 + 1] <= 'f'){
			o2 = 'a' - 10;
		}
		v[i] = ((hex[i * 2] - o1) << 4) + (hex[i * 2 + 1] - o2);
	}
	return  (Vector3){v[0] / 255.0, v[1] / 255.0, v[2] / 255.0};
}

static unsigned int GetTokenLength(JSONObject_t json, unsigned int token){
	return json.tokens[token].end - json.tokens[token].start;
}

static bool CompareToken(JSONObject_t json, unsigned int token, const char *string){
	bool result = false;
	int num_equal = 0;
	if(strlen(string) == GetTokenLength(json, token)){
		for(int i = 0; i < min(GetTokenLength(json, token), strlen(string)); i++){
			if(json.json_string[json.tokens[token].start + i] == string[i]){
				num_equal++;
			}
		}
		if(num_equal == GetTokenLength(json, token)){
			result = true;
		}
	}
	
	return result;
}

static UI_StyleType_et GetPropertyType(JSONObject_t json, unsigned int token){
	UI_StyleType_et value_type = UI_UNDEFINED;
		// px, %, inherit
		if(strncmp(json.json_string + json.tokens[token].end - 2, "px", 2) == 0){
			value_type = UI_PIXELS;
		}else if(json.json_string[json.tokens[token].end - 1] == '%'){
			value_type = UI_PERCENT;
		}else if(strncmp(json.json_string + json.tokens[token].end - 7, "inherit", 7) == 0){
			value_type = UI_STYLE_INHERIT;
		}
	return value_type;
}

static float GetPropertyValue(JSONObject_t json, unsigned int token){
	float value = 0;
		UI_StyleType_et type =  GetPropertyType(json, token);
		unsigned int num_length = GetTokenLength(json, token);
		switch(type){
			case UI_PIXELS:
			case UI_TRANSFORM_PIXELS_INVERTED:
				num_length -= 2; // px
				value = strntol(json.json_string + json.tokens[token].start, num_length);
				break;
			case UI_PERCENT:
				num_length -= 1; // %
				value = strntol(json.json_string + json.tokens[token].start, num_length) / 100.0;
				break;
			default:
				// Either we dont know what it is or it is 'inherit'
				break;
		}
	return value;
}

static int GetPropertyHash(JSONObject_t json, unsigned int token){
	if(json.tokens[token].size > 0 && json.tokens[token].type == JSMN_PRIMITIVE){
		for(int i = 0; i < num_properties; i++){
			if(CompareToken(json, token, property_dict[i])){
				return i;
			}
		}
	}else{
		// Property with no content
		return -2;
	}
	// Unkown property
	return -1;
}

static UIClass *FindClass(UIDomain *domain, char *name, unsigned int length){
	if(name != NULL){
		for(int i = 0; i < domain->num_classes; i++){
			if(length == strlen(domain->classes[i].name)){
				if(strncmp(domain->classes[i].name, name, length) == 0){
					return &domain->classes[i];
				}
			}
		}
	}

	// Class with name 'name' not found
	return NULL;
}

static int SkipToken(JSONObject_t json, unsigned int token){
	// 'token' indexes the name of a property or element
	// 'token + 1' is the data of this property or element
	// and 'token + 2' is the name of the element's first child if it has any children
	// if this element has no children, then 'token + 2' is the name of the next element
	int current_token = token + 1;
	for(int i = 0; i < json.tokens[token].size; i++){
		current_token = SkipToken(json, current_token);
	}
	return current_token;
}

static void LogUnkownToken(JSONObject_t json, unsigned int token, UIDomain *domain){
	if(token <= json.num_tokens){
		char *unkown_property = malloc(GetTokenLength(json, token) + 1);
		strncpy(unkown_property, json.json_string + json.tokens[token].start, GetTokenLength(json, token));
		unkown_property[GetTokenLength(json, token)] = 0;
		printf("%s:%d Unknown property '%s'\n", domain->path, json.tokens[token].start, unkown_property);
		DebugLog(D_WARN, "%s:%d Unknown property '%s'", domain->path, json.tokens[token].start, unkown_property);
		free(unkown_property);
	}
}

static void LoopAction(JSONObject_t json, unsigned int token, UIDomain *domain, UIClass *parent_class, UI_Action act_type){
	UIAction *action = &parent_class->actions[act_type];
	// parent_class->actions = realloc(parent_class->actions, sizeof(UIAction) * (parent_class->num_actions + 1));
	// UIAction *action = parent_class->actions[parent_class->num_actions];

	// Initialize action
	action->enabled = true;
	// action->function = NULL;
	action->classes = NULL;
	action->num_classes = 0;


	unsigned int current_token = token + 2;
	for(int i = 0; i < json.tokens[token + 1].size; i++){

		if(CompareToken(json, current_token, "class")){
			action->classes = realloc(action->classes, sizeof(UIClass *) * (action->num_classes + 1));
			UIClass *class = FindClass(domain, json.json_string + json.tokens[current_token + 1].start, GetTokenLength(json, current_token + 1));
			if(class != NULL){
				action->classes[action->num_classes] = class;
				action->num_classes++;
			// }else{
				// action->classes = realloc(action->classes, sizeof(UIClass *) * (action->num_classes));
				// continue;
			}
			printf("class action\n");
		}else if(CompareToken(json, current_token, "call")){
			printf("call action\n");
		}
		current_token = SkipToken(json, current_token);
	}
}

static int LoopClass(JSONObject_t json, unsigned int token, UIDomain *domain, UIClass *class){
	int name_length = GetTokenLength(json, token);
	class->name = calloc(1, (name_length + 1));
	strncpy(class->name, json.json_string + json.tokens[token].start, name_length);

	// 'token' is the name of the class and 'token + 2' is the first property within the class
	int current_token = token + 2;
	// int current_token = token;

	// bool absolute_position[2] = {false, false};
	float *value_pointer;
	int *type_pointer;
	float value;
	int type;
	bool use_pointers = false;

	for(int i = 0; i < json.tokens[token + 1].size; i++){
		switch(GetPropertyHash(json, current_token)){
			case 0: // position
				break;
			case 1: // top
				value_pointer = &class->transform.y;
				type_pointer = &class->transform_type.y;
				use_pointers = true;
				break;
			case 2: // bottom
				value = GetPropertyValue(json, current_token + 1);
				switch(type = GetPropertyType(json, current_token + 1)){
					case UI_PERCENT:
						value = 1.0 - value;
						break;
					case UI_PIXELS:
						type = UI_TRANSFORM_PIXELS_INVERTED;
					default:
						break;
				}
				class->transform_type.y = type;
				class->transform.y = value;
				break;
			case 3: // left
				value_pointer = &class->transform.x;
				type_pointer = &class->transform_type.x;
				use_pointers = true;
				break;
			case 4: // right
				value = GetPropertyValue(json, current_token + 1);
				switch(type = GetPropertyType(json, current_token + 1)){
					case UI_PERCENT:
						value = 1.0 - value;
						break;
					case UI_PIXELS:
						type = UI_TRANSFORM_PIXELS_INVERTED;
					default:
						break;
				}
				class->transform_type.x = type;
				class->transform.x = value;
				break;

			case 5: // size
				break;
			case 6: // width
				value_pointer = &class->transform.z;
				type_pointer = &class->transform_type.z;
				use_pointers = true;
				break;
			case 7: // height
				value_pointer = &class->transform.w;
				type_pointer = &class->transform_type.w;
				use_pointers = true;
				break;

			case 8: // rotation
				break;

			case 9:;// margin
				value = GetPropertyValue(json, current_token + 1);
				type = GetPropertyType(json, current_token + 1);
				for(int i = 0; i < 4; i++){
					class->margin_type.v[i] = type;
					class->margin.v[i] = value;
				}
				break;
			case 10: // margin-top
				value_pointer = &class->margin.x;
				type_pointer = &class->margin_type.x;
				use_pointers = true;
				break;
			case 11: // margin-right
				value_pointer = &class->margin.y;
				type_pointer = &class->margin_type.y;
				use_pointers = true;
				break;
			case 12: // margin-bottom
				value_pointer = &class->margin.z;
				type_pointer = &class->margin_type.z;
				use_pointers = true;
				break;
			case 13: // margin-left
				value_pointer = &class->margin.w;
				type_pointer = &class->margin_type.w;
				use_pointers = true;
				break;

			case 14:;// border
				value = GetPropertyValue(json, current_token + 1);
				type = GetPropertyType(json, current_token + 1);
				for(int i = 0; i < 4; i++){
					class->border_type.v[i] = type;
					class->border.v[i] = value;
				}
				break;
			case 15: // border-top
				value_pointer = &class->border.x;
				type_pointer = &class->border_type.x;
				use_pointers = true;
				break;
			case 16: // border-right
				value_pointer = &class->border.y;
				type_pointer = &class->border_type.y;
				use_pointers = true;
				break;
			case 17: // border-bottom
				value_pointer = &class->border.z;
				type_pointer = &class->border_type.z;
				use_pointers = true;
				break;
			case 18: // border-left
				value_pointer = &class->border.w;
				type_pointer = &class->border_type.w;
				use_pointers = true;
				break;

			case 19:;// padding
				value = GetPropertyValue(json, current_token + 1);
				type = GetPropertyType(json, current_token + 1);
				for(int i = 0; i < 4; i++){
					class->padding_type.v[i] = type;
					class->padding.v[i] = value;
				}
				break;
			case 20: // padding-top
				value_pointer = &class->padding.x;
				type_pointer = &class->padding_type.x;
				use_pointers = true;
				break;
			case 21: // padding-right
				value_pointer = &class->padding.y;
				type_pointer = &class->padding_type.y;
				use_pointers = true;
				break;
			case 22: // padding-bottom
				value_pointer = &class->padding.z;
				type_pointer = &class->padding_type.z;
				use_pointers = true;
				break;
			case 23: // padding-left
				value_pointer = &class->padding.w;
				type_pointer = &class->padding_type.w;
				use_pointers = true;
				break;

			case 24:;// radius
				value = GetPropertyValue(json, current_token + 1);
				type = GetPropertyType(json, current_token + 1);
				for(int i = 0; i < 4; i++){
					class->radius_type.v[i] = type;
					class->radius.v[i] = value;
				}
				break;
			case 25: // radius-top-left
				value_pointer = &class->radius.x;
				type_pointer = &class->radius_type.x;
				use_pointers = true;
				break;
			case 26: // radius-top-right
				value_pointer = &class->radius.y;
				type_pointer = &class->radius_type.y;
				use_pointers = true;
				break;
			case 27: // radius-bottom-right
				value_pointer = &class->radius.z;
				type_pointer = &class->radius_type.z;
				use_pointers = true;
				break;
			case 28: // radius-bottom-left
				value_pointer = &class->radius.w;
				type_pointer = &class->radius_type.w;
				use_pointers = true;
				break;

			case 29: // opacity
				class->opacity_defined = true;
				value_pointer = &class->opacity;
				type_pointer = NULL;
				use_pointers = true;
				break;
			case 30: // color
				class->color_defined = true;
				class->color = HexToColor(json.json_string + json.tokens[current_token + 1].start);
				break;

			case 31: // border-opacity
				class->border_opacity_defined = true;
				value_pointer = &class->border_opacity;
				type_pointer = NULL;
				use_pointers = true;
				break;
			case 32: // border-color
				class->border_color_defined = true;
				class->border_color = HexToColor(json.json_string + json.tokens[current_token + 1].start);
				break;
			case 33: // text-opacity
				class->text_opacity_defined = true;
				value_pointer = &class->text_opacity;
				type_pointer = NULL;
				use_pointers = true;
				break;
			case 34: // text-color
				class->text_color_defined = true;
				class->text_color = HexToColor(json.json_string + json.tokens[current_token + 1].start);
				break;

			case 37: // align
				// Default align is vertical
				if(CompareToken(json, current_token + 1, "horizontal")){
					class->align = UI_ALIGN_HORIZONTAL;
				}else if(CompareToken(json, current_token + 1, "vertical")){
					class->align = UI_ALIGN_VERTICAL;
				}else{
					class->align = UI_ALIGN_UNDEFINED;
				}
				break;

			case 40: // active
				class->is_active = json.json_string[json.tokens[current_token + 1].start] == 't' ? true : false;
				break;

			case 41: // onhover
				LoopAction(json, current_token, domain, class, UI_ACT_HOVER);
				break;
			case 42: // onenter
				LoopAction(json, current_token, domain, class, UI_ACT_ENTER);
				break;
			case 43: // onleave
				LoopAction(json, current_token, domain, class, UI_ACT_LEAVE);
				break;
			case 44: // onhold
				LoopAction(json, current_token, domain, class, UI_ACT_HOLD);
				break;
			case 45: // onclick
				LoopAction(json, current_token, domain, class, UI_ACT_CLICK);
				break;
			case 46: // onrelease
				LoopAction(json, current_token, domain, class, UI_ACT_RELEASE);
				break;

			case -2:
				DebugLog(D_WARN, "COLON MISSING?");
				LogUnkownToken(json, current_token, domain);
				break;

			default:
				LogUnkownToken(json, current_token, domain);
				break;
		}


		if(use_pointers){
			*value_pointer = GetPropertyValue(json, current_token + 1);
			if(type_pointer != NULL){
				*type_pointer = GetPropertyType(json, current_token + 1);
			}
			use_pointers = false;
		}
		current_token = SkipToken(json, current_token);
	}



	// Return the token after the final token we found
	return current_token;
}

static int LoopClassBuffer(JSONObject_t json, unsigned int token, UIDomain *domain){
	int current_token = token + 2;
	UIClass *class = NULL;
	for(int i = 0; i < json.tokens[token + 1].size; i++){
		class = FindClass(domain, json.json_string + json.tokens[current_token].start, GetTokenLength(json, current_token));
		if(class == NULL){
			// class = &domain->classes[domain->num_classes];
			class = UI_NewClass(domain);

		// }else{
			// domain->num_classes++;

		}
		current_token = LoopClass(json, current_token, domain, class);
		// printf("Class: %s\n", class->name);
	}

	return current_token;
}

// Returns the position of the next element or -1 if there is no next element
static int LoopElement(JSONObject_t json, unsigned int token, UIDomain *domain, UIElement *element){
	int name_length = GetTokenLength(json, token);
	element->name = calloc(1, name_length + 1);
	strncpy(element->name, json.json_string + json.tokens[token].start, name_length);
	element->name[GetTokenLength(json, token)] = 0;

	// 'token' is the name of the element and 'token + 2' is the first property within the element
	int current_token = token + 2;
	for(int i = 0; i < json.tokens[token + 1].size; i++){
		if(CompareToken(json, current_token, "class")){
			if(json.tokens[current_token + 1].type == JSMN_STRING){
				element->classes = realloc(element->classes, sizeof(UIClass) * (element->num_classes + 1));
				element->classes[element->num_classes] = FindClass(domain, json.json_string + json.tokens[current_token + 1].start, GetTokenLength(json, current_token + 1));
				// if(element->classes[element->num_classes] == NULL){
				// 	element->classes = realloc(element->classes, sizeof(UIClass) * (element->num_classes));

				// }else{
				// 	element->num_classes++;

				// }
				if(element->classes[element->num_classes] != NULL){
					element->num_classes++;

				}
			}else{
				DebugLog(D_ERR, "%s:%d Property 'class' must be of string type", domain->path, json.tokens[current_token + 1].start);
			}
			printf("element class\n");
			
		}else if(CompareToken(json, current_token, "text")){
			if(json.tokens[current_token + 1].type == JSMN_STRING){
				element->text = calloc(1, GetTokenLength(json, current_token + 1) + 1);
				strncpy(element->text, json.json_string + json.tokens[current_token + 1].start, GetTokenLength(json, current_token + 1));
				element->text[GetTokenLength(json, current_token + 1)] = 0;
			}else{
				DebugLog(D_ERR, "%s:%d Property 'text' must be of string type", domain->path, json.tokens[current_token + 1].start);
				current_token = SkipToken(json, current_token);
				continue;
			}

		}else if(CompareToken(json, current_token, "onclick")){
			// Set 'current_token' to whatever the 'LoopAction' function returns
			printf("element onclick\n");

		}else if(CompareToken(json, current_token, "elements")){
			if(json.tokens[current_token + 1].type == JSMN_ARRAY){
				int num_elements = json.tokens[current_token + 1].size;
				unsigned int child_token = current_token + 2;
				for(int i = 0; i < num_elements; i++){
					child_token = LoopElement(json, child_token, domain, UI_NewElement(element));
				}
			}else{
				DebugLog(D_ERR, "%s:%d Property 'elements' must be of array type", domain->path, json.tokens[current_token + 1].start);
			}

		}else{
			LogUnkownToken(json, current_token, domain);

		}
		current_token = SkipToken(json, current_token);
	}

	// Return the token after the final token we found
	return current_token;
}

static int LoopBody(JSONObject_t json, unsigned int token, UIDomain *domain){
	// 'token' indexes the token with the string "domain" so the array of elements is in 'token + 1'

	// Start on the name of the first element
	int current_token = token + 2;
	for(int i = 0; i < json.tokens[token + 1].size; i++){
		if(json.tokens[current_token].type == JSMN_STRING){
			current_token = LoopElement(json, current_token, domain, UI_NewElement(&domain->body));
		}
	}

	return current_token;
}

void LoadDomain(char *path, UIDomain *domain){
	JSONObject_t json;

	SDL_RWops *fp = SDL_RWFromFile(path, "r");
	if(fp == NULL){
		printf("Error opening JSON file '%s'", path);
		return;
	}
	long file_length = SDL_RWseek(fp, 0, SEEK_END);
	SDL_RWseek(fp, 0, SEEK_SET);
	if(file_length == -1){
		printf("Error reading length of JSON file '%s'", path);
	}
	printf("file length: %ld\n", file_length);
	json.json_string = malloc(file_length + 1);
	SDL_RWread(fp, json.json_string, 1, file_length);
	
	// Null terminate the copied string
	json.json_string[file_length] = 0;


	// ----- Begin parsing -----

	jsmn_parser parser;

	// Initialize the jsmn parser
	jsmn_init(&parser);

	// Parse once to get number of tokens
	json.num_tokens = jsmn_parse(&parser, json.json_string, strlen(json.json_string), NULL, 0);

	// Error checking
	switch(json.num_tokens){
		case JSMN_ERROR_INVAL:
			printf("JSON error: JSMN_ERROR_INVAL");
			break;
		case JSMN_ERROR_NOMEM:
			printf("JSON error: JSMN_ERROR_NOMEM");
			break;
		case JSMN_ERROR_PART:
			printf("JSON error: JSMN_ERROR_PART");
			break;
	}
	if(json.num_tokens <= 0){
		goto exit;
	}

	// Allocate space for tokens array
	json.tokens = malloc(sizeof(jsmntok_t) * (json.num_tokens + 1));

	// The jsmn parser needs to be re-initialized after having called 'jsmn_parse'
	jsmn_init(&parser);

	// Now actually parse the file and put tokens in token array
	jsmn_parse(&parser, json.json_string, strlen(json.json_string), json.tokens, json.num_tokens);


	domain->path = malloc(sizeof(char) * (strlen(path) + 1));
	strncpy(domain->path, path, strlen(path));
	domain->path[strlen(path)] = 0;


	if(json.tokens[0].type != JSMN_OBJECT){
		DebugLog(D_ERR, "%s:%d UI domain needs to be encapsulated in curly braces", domain->path, json.tokens[0].start);
		goto exit;
	}

	int current_token = 1;
	for(int i = 0; i < json.tokens[0].size; i++){
		if(CompareToken(json, current_token, "classes")){
			current_token = LoopClassBuffer(json, current_token, domain);
		}else if(CompareToken(json, current_token, "body")){
			current_token = LoopBody(json, current_token, domain);
		}else{
			LogUnkownToken(json, current_token, domain);
			current_token = SkipToken(json, current_token);
		}
	}

	exit:
	free(json.tokens);
	json.num_tokens = 0;
	free(active_path);
	SDL_RWclose(fp);
}