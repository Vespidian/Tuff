#include "../global.h"
#include "../debug.h"
#include "ui.h"

void SetElementText(UIElement *element, char *format, ...){
	va_list va_format;
	
	//Use var args to create formatted text
	va_start(va_format, format);
	int length = vsnprintf(NULL, 0, format, va_format);
	char *formatted_text = malloc(length + 1);
	vsnprintf(formatted_text, length + 1, format, va_format);
	va_end(va_format);

	element->text = malloc(length + 1);
	if(element->text != NULL){
		strcpy(element->text, formatted_text);
		element->text[length] = 0;
	}

	free(formatted_text);
}

UIClass *FindClass(UIScene *scene, char *name){
	for(int i = 0; i < scene->num_classes; i++){
		if(strcmp(scene->classes[i].name, name) == 0){
			return &scene->classes[i];
		}
	}
	return NULL;
}

static UIElement *RecursiveFindElement(UIElement *element, char *name){
	if(strcmp(element->name, name) == 0){
		return element;
	}
	UIElement *child = NULL;
	for(int i = 0; i < element->num_children; i++){
		if((child = RecursiveFindElement(&element->children[i], name)) != NULL){
			return child;
		}
	}
	return NULL;
}

UIElement *FindElement(UIScene *scene, char *name){
	UIElement *element = RecursiveFindElement(&scene->body, name);
	return element;
}