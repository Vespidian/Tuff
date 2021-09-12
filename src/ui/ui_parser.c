#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <SDL.h>

#include "../global.h"
#include "../include/jsmn.h"
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

const int num_properties = 43;
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
	"border-color",
	"text-color",
	"text-size",
	"font",

	"transition-duration",
	"transition-ease",

	"onhover",
	"onenter",
	"onleave",
	"onhold",
	"onclick",
	"onrelease",

}; // MAKE SURE to update 'num_properties' when editing the number of properties

static int min(int a, int b){
	int result = a;
	if(b < a){
		result = b;
	}
	return result;
}

UIClass *UI_NewClass(UIScene *scene){
	scene->classes = realloc(scene->classes, sizeof(UIClass) * ++scene->num_classes);
	UIClass *class = &scene->classes[scene->num_classes - 1];

	class->name = 0;
	class->font_defined = false;
	class->text_size_defined = false;
	class->color_defined = false;
	class->border_color_defined = false;
	class->text_color_defined = false;
	class->transition_defined = false;
	class->origin = UI_ORIGIN_TOP_LEFT;

	class->transform_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->transform_relative_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->margin_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->border_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->padding_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};
	class->radius_type = (UI_Property){UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED, UI_UNDEFINED};

	class->ease = UI_EASE_UNDEFINED;

	for(int i = 0; i < UI_NUM_ACTIONS; i++){
		class->actions[i].num_classes = 0;
		class->actions[i].function = NULL;
	}

	return class;
}

UIElement *UI_NewElement(UIElement *parent){
	parent->children = realloc(parent->children, sizeof(UIElement) * ++parent->num_children);
	UIElement *element = &parent->children[parent->num_children - 1];

	element->name = 0;

	element->parent = parent;
	element->children = NULL;
	element->num_children = 0;

	element->image = false;

	element->text = 0;
	element->font = &default_font;
	element->text_size = 1;
	element->text_color = (Vector4){1, 1, 1, 1};

	element->color = (Vector4){1, 1, 1, 1};
	element->border_color = (Vector4){0, 0, 0, 1};

	element->base_position= (Vector2){0, 0};
	element->base_scale = (Vector2){0, 0};
	element->transform 	= (Vector4){0, 0, 50, 50};
	element->margin 	= (Vector4){10, 10, 10, 10};
	element->border 	= (Vector4){1, 1, 1, 1};
	element->padding 	= (Vector4){10, 10, 10, 10};
	element->radius 	= (Vector4){0, 0, 0, 0};

	element->ease_position = 0;

	element->classes = NULL;
	element->num_classes = 0;

	element->is_selected = false;
	element->is_active = true;

	return element;
}

void InitializeScene(UIScene *scene){
	scene->classes = NULL;
	scene->num_classes = 0;

	scene->dynamic = false; // Scenes are static by default

	scene->needs_update = true;


	scene->body.name = malloc(sizeof(char) * 5);
	strcpy(scene->body.name, "body");

	scene->body.parent = NULL;
	scene->body.children = NULL;
	scene->body.num_children = 0;

	scene->body.image = false;

	scene->body.text = 0;
	scene->body.font = &default_font;
	scene->body.text_size = 1;
	scene->body.text_color = (Vector4){1, 1, 1, 1};

	scene->body.color = (Vector4){0, 0, 0, 0};
	scene->body.border_color = (Vector4){0, 0, 0, 1};

	scene->body.base_position= (Vector2){0, 0};
	scene->body.base_scale 	= (Vector2){SCREEN_WIDTH, SCREEN_HEIGHT};
	scene->body.transform 	= (Vector4){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	scene->body.margin 		= (Vector4){5, 5, 5, 5};
	scene->body.border 		= (Vector4){0, 0, 0, 0};
	scene->body.padding 	= (Vector4){10, 10, 10, 10};
	scene->body.radius 		= (Vector4){0, 0, 0, 0};

	scene->body.full_screen = true;

	scene->body.ease_position = 0;

	scene->body.classes = NULL;
	scene->body.num_classes = 0;

	scene->body.is_selected = false;
	scene->body.is_active = true;
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

static bool CompareToken(JSONObject_t json, unsigned int token, const char *string){ // TODO: convert to these parameters
// static bool CompareToken(jsmntok_t *token, char *json_string, char *string){
	bool result = false;
	int num_equal = 0;
	if(strlen(string) == json.tokens[token].end - json.tokens[token].start){
		for(int i = 0; i < min(json.tokens[token].end - json.tokens[token].start, strlen(string)); i++){
			if(json.json_string[json.tokens[token].start + i] == string[i]){
				num_equal++;
			}
		}
		if(num_equal == json.tokens[token].end - json.tokens[token].start){
			result = true;
		}
	}

	return result;
}

static unsigned int GetTokenLength(JSONObject_t json, unsigned int token){
	return json.tokens[token].end - json.tokens[token].start;
}

static UI_StyleType_et GetPropertyType(JSONObject_t json, unsigned int token){
	// px, %, inherit
	UI_StyleType_et value_type = UI_UNDEFINED;
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
	UI_StyleType_et type =  GetPropertyType(json, token);
	unsigned int num_length = GetTokenLength(json, token);
	float value = 0;
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
	for(int i = 0; i < num_properties; i++){
		if(CompareToken(json, token, property_dict[i])){
			return i;
		}
	}
	// Unkown property
	return -1;
}


static UIClass *FindClass(UIScene *scene, char *name, unsigned int length){
	for(int i = 0; i < scene->num_classes; i++){
		if(strncmp(scene->classes[i].name, name, length) == 0){
			return &scene->classes[i];
		}
	}
	return NULL;
}

static int SkipToken(JSONObject_t json, unsigned int token){
	// 'token' indexes the name of a property or element
	// 'token + 1' is the data of this property or element
	// and 'token + 2' is the name of the element's first child if it has any children
	// if this element has no children, then 'token + 2' is the name of the next element
	int current_token = token + 2;
	for(int i = 0; i < json.tokens[token + 1].size; i++){
		current_token = SkipToken(json, current_token);
	}
	return current_token;
}

static void LogUnkownToken(JSONObject_t json, unsigned int token, UIScene *scene){
	char *unkown_property = malloc(GetTokenLength(json, token) + 1);
	strncpy(unkown_property, json.json_string + json.tokens[token].start, GetTokenLength(json, token));
	unkown_property[GetTokenLength(json, token)] = 0;
	// printf("UISS: Unknown property (): %s\n", unkown_property);
	printf("%s:%d Unknown property '%s'\n", scene->path, json.tokens[token].start, unkown_property);
	// DebugLog(D_WARN, "UISS: Unknown property: %s", unkown_property);
	DebugLog(D_WARN, "%s:%d Unknown property '%s'", scene->path, json.tokens[token].start, unkown_property);
	free(unkown_property);
}

static int LoopClass(JSONObject_t json, unsigned int token, UIScene *scene, UIClass *class){
	int name_length = GetTokenLength(json, token);
	class->name = calloc(sizeof(char), (name_length + 1));
	strncpy(class->name, json.json_string + json.tokens[token].start, name_length);

	// 'token' is the name of the class and 'token + 2' is the first property within the class
	int current_token = token + 2;

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
				break;
			case 11: // margin-right
				break;
			case 12: // margin-bottom
				break;
			case 13: // margin-left
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


			default:
				LogUnkownToken(json, current_token, scene);
				break;
		}

		if(use_pointers){
			*value_pointer = GetPropertyValue(json, current_token + 1);
			// if(type_override != -1){ // If the override is not -1 that means it is set
			// 	*type_pointer = type_override;
			// 	type_override = -1;
			// }else{
				*type_pointer = GetPropertyType(json, current_token + 1);
			// }
			use_pointers = false;
		}
		current_token = SkipToken(json, current_token);
	}

	// Return the token after the final token we found
	return current_token;
}

static int LoopClassBuffer(JSONObject_t json, unsigned int token, UIScene *scene){
	// scene->num_classes = json.tokens[token + 1].size;
	// scene->classes = malloc(sizeof(UIClass) * (scene->num_classes + 1));

	int current_token = token + 2;
	for(int i = 0; i < json.tokens[token + 1].size; i++){
		// current_token = LoopClass(json, current_token, scene, &scene->classes[i]);
		current_token = LoopClass(json, current_token, scene, UI_NewClass(scene));
	}

	return current_token;
}

// Returns the position of the next element or -1 if there is no next element
static int LoopElement(JSONObject_t json, unsigned int token, UIScene *scene, UIElement *element){
	int name_length = GetTokenLength(json, token);
	element->name = malloc(sizeof(char) * (name_length + 1));
	strncpy(element->name, json.json_string + json.tokens[token].start, name_length);

	// 'token' is the name of the element and 'token + 2' is the first property within the element
	int current_token = token + 2;
	for(int i = 0; i < json.tokens[token + 1].size; i++){
		if(CompareToken(json, current_token, "class")){
			element->classes = realloc(element->classes, sizeof(UIClass) * (element->num_classes + 1));
			element->classes[element->num_classes] = FindClass(scene, json.json_string + json.tokens[current_token + 1].start, GetTokenLength(json, current_token + 1));
			element->num_classes++;
			printf("element class\n");
			current_token += 2;
			
		}else if(CompareToken(json, current_token, "onclick")){
			// Set 'current_token' to whatever the 'LoopAction' function returns
			current_token = SkipToken(json, current_token);
			printf("element onclick\n");

		}else if(CompareToken(json, current_token, "elements")){
			element->num_children = json.tokens[current_token + 1].size;
			element->children = malloc(sizeof(UIElement) * (element->num_children + 1));

			for(int i = 0; i < element->num_children; i++){
				current_token = LoopElement(json, current_token, scene, &element->children[i]);
			}

		}else{
			LogUnkownToken(json, current_token, scene);
			current_token = SkipToken(json, current_token);

		}
	}

	// Return the token after the final token we found
	return current_token;
}

static int LoopBody(JSONObject_t json, unsigned int token, UIScene *scene){
	// 'token' indexes the token with the string "scene" so the array of elements is in 'token + 1'
	// scene->body.num_children = json.tokens[token + 1].size;
	// scene->body.children = malloc(sizeof(UIElement) * (scene->body.num_children + 1));

	// Start on the name of the first element
	int current_token = token + 2;
	for(int i = 0; i < json.tokens[token + 1].size; i++){
		current_token = LoopElement(json, current_token, scene, UI_NewElement(&scene->body));
		// current_token = LoopElement(json, current_token, scene, &scene->body.children[i]);
	}
	printf("name after last element: %d\n", current_token);

	return current_token;
}

void LoadScene(char *path, UIScene *scene){
	JSONObject_t json;

	SDL_RWops *fp = SDL_RWFromFile(path, "r");
	if(fp == NULL){
		printf("Error opening JSON file '%s'", path);
	}
	long file_length = SDL_RWseek(fp, 0, SEEK_END);
	SDL_RWseek(fp, 0, SEEK_SET);
	if(file_length == -1){
		printf("Error reading length of JSON file '%s'", path);
	}
	printf("file length: %ld\n", file_length);
	json.json_string = malloc(sizeof(char) * (file_length + 1));
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

	// Allocate space for tokens array
	json.tokens = malloc(sizeof(jsmntok_t) * (json.num_tokens + 1));

	// The jsmn parser needs to be re-initialized after having called 'jsmn_parse'
	jsmn_init(&parser);

	// Now actually parse the file and put tokens in token array
	jsmn_parse(&parser, json.json_string, strlen(json.json_string), json.tokens, json.num_tokens);




	// char *new_string;
	
	// Allocate space for new string
	// for(int i = 0; i < json.num_tokens; i++){
	// 	new_string = calloc((json.tokens[i].end - json.tokens[i].start + 1), sizeof(char));
		
	// 	// Copy section of string
	// 	memcpy(new_string, json.json_string + json.tokens[i].start, json.tokens[i].end - json.tokens[i].start);
		
	// 	// Null terminate the copied string
	// 	new_string[json.tokens[i].end - json.tokens[i].start] = 0;
	// 	printf("Token %d:\n%s\n\n", i, new_string);
	// 	// printf("Token %d type:\n%d\n\n", i, json.tokens[i].type);
	// }
	// free(new_string);

	// CompareToken(&json.tokens[13], json.json_string, "class");

	if(json.tokens[0].type != JSMN_OBJECT){
		DebugLog(D_ERR, "Could not parse UI file '%s'. Entire scene should be encompased by curly braces '{}'", path);
	}

	// active_path = malloc(sizeof(char) * (strlen(path) + 1));
	// strncpy(active_path, path, strlen(path));


	scene->path = malloc(sizeof(char) * (strlen(path) + 1));
	strncpy(scene->path, path, strlen(path));
	scene->path[strlen(path)] = 0;

	// CurrentObject_et current_object = UNKOWN_OBJECT;
	// UIClass *current_class = NULL;
	// UIElement *current_element = NULL;
	// UI_Action current_action;
	// unsigned int json_depth = 0;
	// for(int i = 0; i < json.num_tokens; i++){
	int current_token = 1;
	for(int i = 0; i < json.tokens[0].size; i++){
		if(CompareToken(json, current_token, "classes")){
			current_token = LoopClassBuffer(json, current_token, scene);
		}else if(CompareToken(json, current_token, "body")){
			current_token = LoopBody(json, current_token, scene);
		}else{
			LogUnkownToken(json, current_token, scene);
			current_token = SkipToken(json, current_token);
		}

		// switch(current_object){
		// 	case UNKOWN_OBJECT:
		// 		break;
		// 	case CLASS_BUFFER_OBJECT:
		// 		if(json.tokens[i].type == JSMN_STRING){
		// 			current_object = CLASS_OBJECT;
		// 		}
		// 		break;
		// 	case SCENE_OBJECT:
		// 		if(json.tokens[i].type == JSMN_STRING){
		// 			current_object = ELEMENT_OBJECT;
		// 		}
		// 		break;
		// 	case CLASS_OBJECT:
		// 		if(CompareToken(json.tokens[i], "position")){
		// 			current_object = PROPERTY_OBJECT;
		// 		}else if(CompareToken(json.tokens[i], "color")){
		// 			current_object = PROPERTY_OBJECT;
		// 		}
		// 		break;
		// 	case ELEMENT_OBJECT:
		// 		if(CompareToken(json.tokens[i], "class")){
		// 			current_object = CLASS_OBJECT;
		// 		}else if(CompareToken(json.tokens[i], "onclick")){
		// 			current_object = EVENT_OBJECT;
		// 			current_action = UI_ACT_CLICK;
		// 		}
		// 		break;
		// 	case PROPERTY_OBJECT:
		// 		break;
		// 	case EVENT_OBJECT:
		// 		break;
		// }

	}
printf("%d\n", GetPropertyHash(json, 5));

	free(active_path);

	// printf("%d\n", json.tokens[12].size);
	// printf("%d tokens\n", json.num_tokens);
	// printf("%d\n", json.tokens[3].size);

}