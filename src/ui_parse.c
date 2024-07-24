#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// #include <jsmn.h>

#include "vectorlib.h"
#include "json_read.h"
#include "debug.h"
#include "ui.h"

extern UIClass UIDefaultRootClass();
extern UIElement *UINewElement(UIState *state);
extern void UIElementAddChild(UIElement *parent, UIElement *child);
extern UIClass *UINewClass(UIState *state);


static UIState *ui_state_ptr = NULL;
static UIClass *class_ptr = NULL;
static UIElement *element_ptr = NULL;


static char *boolean_dict[] = {
	"false",
	"true",
	NULL
};
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

#define PADDING_INDEX 2
#define BORDER_INDEX 12
#define MARGIN_INDEX 7
static char *class_attributes[] = {
	"color",
	"border-color",

	"padding",
	"padding-top",
	"padding-bottom",
	"padding-left",
	"padding-right",

	"margin",
	"margin-top",
	"margin-bottom",
	"margin-left",
	"margin-right",

	"border",
	"border-top",
	"border-bottom",
	"border-left",
	"border-right",

	"max-width",
	"max-height",
	"min-width",
	"min-height",
	"width",
	"height",

	"wrap",
	"wrap-vertical",
	"wrap-reverse",
	"cull",
	"inherit",
	"origin-c",
	"origin-p",

	"on-hold",
	"on-hover",

	NULL
};
static char *origin_names[] = {
	"top-left",
	"top",
	"top-right",
	"left",
	"center",
	"right",
	"bottom-left",
	"bottom",
	"bottom-right",

	NULL
};
static void tfunc2_classes_attributes(JSONState *json, unsigned int token){
	// Make switch case here with every class attribute
	int val = -1;
	char *str = NULL;
	switch(JSONTokenHash(json, token, class_attributes)){
		case 0: // color
			JSONTokenToString(json, token + 1, &str);
			Vector3 color = HexToColor(str);
			class_ptr->color = color;
			break;
		case 1: // border-color
			JSONTokenToString(json, token + 1, &str);
			Vector3 border_color = HexToColor(str);
			class_ptr->border_color = border_color;
			break;

		case PADDING_INDEX: // padding
			val = JSONTokenValue(json, token + 1)._int;
			class_ptr->padding.x = val;
			class_ptr->padding.y = val;
			class_ptr->padding.z = val;
			class_ptr->padding.w = val;
			break;
		case PADDING_INDEX + 1: // padding-top
			class_ptr->padding.y = JSONTokenValue(json, token + 1)._int;
			break;
		case PADDING_INDEX + 2: // padding-bottom
			class_ptr->padding.w = JSONTokenValue(json, token + 1)._int;
			break;
		case PADDING_INDEX + 3: // padding-left
			class_ptr->padding.x = JSONTokenValue(json, token + 1)._int;
			break;
		case PADDING_INDEX + 4: // padding-right
			class_ptr->padding.z = JSONTokenValue(json, token + 1)._int;
			break;


		case MARGIN_INDEX: // margin
			val = JSONTokenValue(json, token + 1)._int;
			class_ptr->margin.x = val;
			class_ptr->margin.y = val;
			class_ptr->margin.z = val;
			class_ptr->margin.w = val;
			break;
		case MARGIN_INDEX + 1: // margin-top
			class_ptr->margin.y = JSONTokenValue(json, token + 1)._int;
			break;
		case MARGIN_INDEX + 2: // margin-bottom
			class_ptr->margin.w = JSONTokenValue(json, token + 1)._int;
			break;
		case MARGIN_INDEX + 3: // margin-left
			class_ptr->margin.x = JSONTokenValue(json, token + 1)._int;
			break;
		case MARGIN_INDEX + 4: // margin-right
			class_ptr->margin.z = JSONTokenValue(json, token + 1)._int;
			break;


		case BORDER_INDEX: // border
			val = JSONTokenValue(json, token + 1)._int;
			class_ptr->border.x = val;
			class_ptr->border.y = val;
			class_ptr->border.z = val;
			class_ptr->border.w = val;
			break;
		case BORDER_INDEX + 1: // border-top
			class_ptr->border.y = JSONTokenValue(json, token + 1)._int;
			break;
		case BORDER_INDEX + 2: // border-bottom
			class_ptr->border.w = JSONTokenValue(json, token + 1)._int;
			break;
		case BORDER_INDEX + 3: // border-left
			class_ptr->border.x = JSONTokenValue(json, token + 1)._int;
			break;
		case BORDER_INDEX + 4: // border-right
			class_ptr->border.z = JSONTokenValue(json, token + 1)._int;
			break;


		case 17: // max-width
			if(json->json_string[json->tokens[token + 1].end - 1] == '%'){
				class_ptr->size_max_percent.x = strntol(json->json_string + json->tokens[token + 1].start, json->tokens[token + 1].end - json->tokens[token + 1].start - 1);
			}else{
				class_ptr->size_max.x = JSONTokenValue(json, token + 1)._int;
			}

			break;
		case 18: // max-height
			if(json->json_string[json->tokens[token + 1].end - 1] == '%'){
				class_ptr->size_max_percent.y = strntol(json->json_string + json->tokens[token + 1].start, json->tokens[token + 1].end - json->tokens[token + 1].start - 1);
			}else{
				class_ptr->size_max.y = JSONTokenValue(json, token + 1)._int;
			}
			break;
		case 19: // min-width
			if(json->json_string[json->tokens[token + 1].end - 1] == '%'){
				class_ptr->size_min_percent.x = strntol(json->json_string + json->tokens[token + 1].start, json->tokens[token + 1].end - json->tokens[token + 1].start - 1);
			}else{
				class_ptr->size_min.x = JSONTokenValue(json, token + 1)._int;
			}
			break;
		case 20: // min-height
			if(json->json_string[json->tokens[token + 1].end - 1] == '%'){
				class_ptr->size_min_percent.y = strntol(json->json_string + json->tokens[token + 1].start, json->tokens[token + 1].end - json->tokens[token + 1].start - 1);
			}else{
				class_ptr->size_min.y = JSONTokenValue(json, token + 1)._int;
			}
			break;

		case 21: // width
			if(json->json_string[json->tokens[token + 1].end - 1] == '%'){
				class_ptr->size_max_percent.x = strntol(json->json_string + json->tokens[token + 1].start, json->tokens[token + 1].end - json->tokens[token + 1].start - 1);
				class_ptr->size_min_percent.x = strntol(json->json_string + json->tokens[token + 1].start, json->tokens[token + 1].end - json->tokens[token + 1].start - 1);
			}else{
				class_ptr->size_max.x = JSONTokenValue(json, token + 1)._int;
				class_ptr->size_min.x = JSONTokenValue(json, token + 1)._int;
			}
			break;
		case 22: // height
			if(json->json_string[json->tokens[token + 1].end - 1] == '%'){
				class_ptr->size_max_percent.y = strntol(json->json_string + json->tokens[token + 1].start, json->tokens[token + 1].end - json->tokens[token + 1].start - 1);
				class_ptr->size_min_percent.y = strntol(json->json_string + json->tokens[token + 1].start, json->tokens[token + 1].end - json->tokens[token + 1].start - 1);
			}else{
				class_ptr->size_max.y = JSONTokenValue(json, token + 1)._int;
				class_ptr->size_min.y = JSONTokenValue(json, token + 1)._int;
			}
			break;

		case 23: // wrap
			break;
		case 24: // wrap-vertical
			class_ptr->wrap_vertical = JSONTokenHash(json, token + 1, boolean_dict);
			break;
		case 25: // wrap-reverse
			class_ptr->wrap_reverse = JSONTokenHash(json, token + 1, boolean_dict);
			break;
		case 26: // cull
			break;
		case 27: // inherit
			break;
		case 28: // origin-c
			class_ptr->origin_c = JSONTokenHash(json, token + 1, origin_names);
			break;
		case 29: // origin-p
			class_ptr->origin_p = JSONTokenHash(json, token + 1, origin_names);
			break;


		case 30: // on-hold
			class_ptr->class_hold = NULL;
			JSONTokenToString(json, token + 1, &class_ptr->class_hold);
			break;
		case 31: // on-hover
			class_ptr->class_hover = NULL;
			JSONTokenToString(json, token + 1, &class_ptr->class_hover);
			break;


		default:
			JSONTokenToString(json, token, &str);
			DebugLog(D_WARN, "warning: %s: unknown attribute '%s'", json->path, str);
			break;
	}
	free(str);
	str = NULL;
}

static void tfunc1_classes(JSONState *json, unsigned int token){
	JSONToken json_token = JSONTokenValue(json, token);
	if(json_token.type == JSON_STRING){
		class_ptr = UINewClass(ui_state_ptr);
		JSONTokenToString(json, token, &class_ptr->name);

		JSONSetTokenFunc(json, NULL, tfunc2_classes_attributes);
		JSONParse(json);
	}
}

static void tfunc1_new_elements(JSONState *json, unsigned int token);
static char *element_attributes[] = {
	"name",
	"class",
	"text",
	"children",
	"texture",

	NULL
};
static void tfunc2_element_data(JSONState *json, unsigned int token){
	switch(JSONTokenHash(json, token, element_attributes)){
		case 0: // name
			JSONTokenToString(json, token + 1, &element_ptr->name);

			break;
		case 1: // class
			char *name = NULL;
			JSONTokenToString(json, token + 1, &name);
			UIElementAddClass(element_ptr, UIFindClass(ui_state_ptr, name));
			free(name);
			name = NULL;

			break;
		case 2: // text
			JSONTokenToString(json, token + 1, &element_ptr->text);

			break;
		case 3: // children
			JSONSetTokenFunc(json, NULL, tfunc1_new_elements);
			JSONParse(json);

			break;
		case 4: // texture
			char *str = NULL;
			JSONTokenToString(json, token + 1, &str);
			element_ptr->texture = TextureOpen(str, TEXTURE_FILTERING_NEAREST);
			free(str);
			str = NULL;
			break;
		default: break;
	}
}

static void tfunc1_new_elements(JSONState *json, unsigned int token){
	// JSONToken json_token = JSONTokenValue(json, token);

	if(json->tokens[token].type == JSMN_OBJECT){
		UIElement *element = UINewElement(ui_state_ptr);

		UIElementAddChild(element_ptr, element);

		element_ptr = element;
		// JSONTokenToString(json, token, &element_ptr->name);
		JSONSetTokenFunc(json, NULL, tfunc2_element_data);
		JSONParse(json);

		element_ptr = element->parent;

	}else if(JSONTokenHash(json, token, element_attributes) == 1){ // class
		char *name = NULL;
		JSONTokenToString(json, token + 1, &name);
		UIElementAddClass(element_ptr, UIFindClass(ui_state_ptr, name));
		free(name);
		name = NULL;
	}

}

static char *root_dict[] = {
	"classes",
	"elements",
	NULL
};
static void tfunc0_default(JSONState *json, unsigned int token){
	switch(JSONTokenHash(json, token, root_dict)){
		case 0: // classes
			JSONSetTokenFunc(json, NULL, tfunc1_classes);
			JSONParse(json);

			break;
		case 1: // elements
			JSONSetTokenFunc(json, NULL, tfunc1_new_elements);
			JSONParse(json);

			break;
		default: break;
	}
}
/* ---  ^ DEEPER NESTED AS WE GO UP ^  --- */
/* --- JSON ROOT - PARSING STARTS HERE --- */


void UIParse(UIState *state, char *path){
	*state = UINewState();
	ui_state_ptr = state;

	if(path != NULL){
		ui_state_ptr->path = malloc(strlen(path));
		memcpy(ui_state_ptr->path, path, strlen(path));

		// Initialize root element
		element_ptr = UINewElement(state);
		element_ptr->class = UIDefaultRootClass();


		JSONState json = JSONOpen(path);
		JSONSetTokenFunc(&json, NULL, tfunc0_default);
		JSONParse(&json);
		JSONFree(&json);


	}else{
		DebugLog(D_WARN, "UI path string is null");
	}

	ui_state_ptr = NULL;
	element_ptr = NULL;
	class_ptr = NULL;
}
