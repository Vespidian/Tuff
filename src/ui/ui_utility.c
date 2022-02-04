#include "../global.h"
#include "../debug.h"
#include "ui.h"

void SetElementText(UIElement *element, char *format, ...){
	if(element != NULL && format != NULL){
		va_list va_format;
		va_list copy;
		
		//Use var args to create formatted text
		va_start(va_format, format);
		va_copy(copy, va_format);
		int length = vsnprintf(NULL, 0, format, copy);
		va_end(copy);
		char *formatted_text = malloc(length + 1);

		if(formatted_text != NULL){
			vsnprintf(formatted_text, length + 1, format, va_format);
			formatted_text[length] = 0;

			element->text = malloc(length + 1);
			if(element->text != NULL){
				strcpy(element->text, formatted_text);
				element->text[length] = 0;
			}
			free(formatted_text);
		}
		va_end(va_format);
	}
}

UIClass *FindClass(UIDomain *domain, char *name){
	if(name != NULL){
		for(int i = 0; i < domain->num_classes; i++){
			if(domain->classes[i].name == NULL){
				printf("OH NO\n");
			}
			if(strcmp(domain->classes[i].name, name) == 0){
				return &domain->classes[i];
			}
		}
	}
	return NULL;
}

static UIElement *RecursiveFindElement(UIElement *element, char *name){
	UIElement *child = NULL;
	if(name != NULL && element != NULL){
		if(strcmp(element->name, name) == 0){
			child = element;

		}else{
			for(int i = 0; i < element->num_children; i++){
				if((child = RecursiveFindElement(&element->children[i], name)) != NULL){
					break;
					
				}
			}
		}
	}
	return child;
}

UIElement *FindElement(UIDomain *domain, char *name){
	UIElement *element = RecursiveFindElement(&domain->body, name);
	return element;
}