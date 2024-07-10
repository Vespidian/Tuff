/**
 * Pimy - 29/08/2023
 * JSONWrite Serializer
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "json_write.h"

const unsigned int resize_padding = 100;

static size_t len(const char *s){
	int i = 0;
	for(; s[i] != '\0'; i++);
	return i;
}

static void JSONConcat(JSONWrite *json, const char *str){
	if(json != NULL && str != NULL){
		
		// If the string doesnt fit, resize the contents array
		if(len(json->contents) < (json->content_size + len(str))){
			char *tmp = realloc(json->contents, json->content_size + len(str) + resize_padding);
			if(tmp != NULL){
				json->contents = tmp;
				json->content_size += len(str) + resize_padding;
			}else{
				printf("error resizing json contents array\n");
				return;
			}
		}

		// Add the string to the contents array
		strncat(json->contents, str, len(str));
	}
}

void JSONWriteIndent(JSONWrite *json){
	if(json != NULL){

		json->depth++;
		
		// Add 'json.previous_token' object to 'json.hierarchy'
		JSON_WRITE_TYPE *tmp = realloc(json->hierarchy, sizeof(JSON_WRITE_TYPE) * (json->depth + 2));
		if(tmp != NULL){
			json->hierarchy = tmp;
		}else{
			printf("error resizing json hierarchy array\n");
			return;
		}
		json->hierarchy[json->depth - 1] = json->previous_token;

		// Allow for empty objects
		if(json->previous_token != JSON_WRITE_TYPE_UNDEFINED && json->previous_token != JSON_WRITE_TYPE_ARRAY && json->previous_token != JSON_WRITE_TYPE_OBJECT){
			// Prepend x indents where x = (json->depth - 1)
			for(int i = 0; i < json->depth - 1; i++){
				JSONConcat(json, "	");
			}
		}

		// Append a '{' or '[' and a newline
		char symbol[6] = "{\n\0";
		if(json->previous_token == JSON_WRITE_TYPE_ARRAY){
			symbol[0] = '[';
		}
		JSONConcat(json, symbol);

	}
}

void JSONWriteOutdent(JSONWrite *json){
	if(json != NULL){

		// Only do anything if the current object is not root
		if(json->depth > 0){

			json->depth--;

			// Prepend x indents where x = (json->depth - 1)
			for(int i = 0; i < json->depth ; i++){
				JSONConcat(json, "	");
			}

			// Print out the closing bracket '}' or ']' and comma and a newline
			char symbol[6] = "},\n\0";
			if(json->hierarchy[json->depth] == JSON_WRITE_TYPE_ARRAY){
				symbol[0] = ']';
			}
			JSONConcat(json, symbol);

			// Pop the most recent object from the 'json.hierarchy'
			json->hierarchy[json->depth] = JSON_WRITE_TYPE_UNDEFINED;

		}
	}
}

void JSONWriteElement(JSONWrite *json, JSON_WRITE_TYPE type, const char *element_name, const void *value){

	// Escaping invalid elements
	if(type == JSON_WRITE_TYPE_ARRAY || type == JSON_WRITE_TYPE_OBJECT){
		if(value != NULL){
			return;
		}
	}else if(type != JSON_WRITE_TYPE_UNDEFINED){
		if(value == NULL){
			return;
		}
	}

	// Check if previous token is an array or object
	if(json->previous_token == JSON_WRITE_TYPE_ARRAY || json->previous_token == JSON_WRITE_TYPE_OBJECT){
		// If the array / object is not indented, then we signify it is empty with brackets
		if(json->contents[len(json->contents)] == ':'){
			char brackets[6] = "{},\n\0";
			if(json->previous_token == JSON_WRITE_TYPE_ARRAY){
				brackets[0] = '[';
				brackets[1] = ']';
			}

			JSONConcat(json, brackets);
		}
	}

	// Set previous token to this token
	json->previous_token = type;

	// Print out the data specified with x prepended indents where x = json->depth
	for(int i = 0; i < json->depth; i++){
		JSONConcat(json, "	");
	}
	if(element_name != NULL){
		JSONConcat(json, "\"");
		JSONConcat(json, element_name);
		JSONConcat(json, "\":");
	}

	char str_num[32];
	switch(type){
		case JSON_WRITE_TYPE_FLOAT:
			sprintf(str_num, "%f", *(float *)value);
			JSONConcat(json, str_num);
			JSONConcat(json, ",\n");
			break;
		case JSON_WRITE_TYPE_INT:
			sprintf(str_num, "%d", *(int *)value);
			JSONConcat(json, str_num);
			JSONConcat(json, ",\n");
			break;
		case JSON_WRITE_TYPE_STRING:
			JSONConcat(json, "\"");
			JSONConcat(json, value);
			JSONConcat(json, "\",\n");
			break;
		case JSON_WRITE_TYPE_BOOLEAN:
			JSONConcat(json, *(bool *)value ? "true,\n" : "false,\n");
			break;
		case JSON_WRITE_TYPE_OBJECT:
			break;
		case JSON_WRITE_TYPE_ARRAY:
			break;
		default:
			break;
	}

}

JSONWrite JSONWriteNew(){
	JSONWrite json;

	json.file = NULL;
	json.contents = calloc(resize_padding, 1);
	json.content_size = resize_padding;
	json.depth = 0;

	json.previous_token = JSON_WRITE_TYPE_UNDEFINED;

	json.hierarchy = malloc(sizeof(JSON_WRITE_TYPE) * 2);
	json.hierarchy[0] = JSON_WRITE_TYPE_UNDEFINED;

	return json;
}

void JSONWriteToFile(JSONWrite *json, char *path){
	if(json != NULL && path != NULL){

		// Outdent to root in case we are still inside an object / array
		// This guarantees the production of a valid json file
		for(int i = 0; i < json->depth; i++){
			JSONWriteOutdent(json);
		}

		json->file = fopen(path, "w");

		if(json->file != NULL){

			fputs(json->contents, json->file);
			fclose(json->file);

		}else{

			printf("Error opening file '%s'", path);

		}
	}
}

void JSONWriteFree(JSONWrite *json){

	free(json->contents);
	json->contents = NULL;

	free(json->hierarchy);
	json->hierarchy = NULL;

	json->previous_token = JSON_WRITE_TYPE_UNDEFINED;

}