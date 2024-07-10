/**
 * Pimy - 29/08/2023
 * JSONWrite Serializer
*/

#ifndef JSON_WRITE_H_
#define JSON_WRITE_H_

typedef enum JSON_WRITE_TYPE {
	JSON_WRITE_TYPE_UNDEFINED,
	JSON_WRITE_TYPE_INT, 
	JSON_WRITE_TYPE_FLOAT, 
	JSON_WRITE_TYPE_STRING, 
	JSON_WRITE_TYPE_BOOLEAN, 
	JSON_WRITE_TYPE_OBJECT, 
	JSON_WRITE_TYPE_ARRAY
}JSON_WRITE_TYPE;

typedef struct JSONWrite{
	FILE *file;
	
    unsigned int content_size;
	char *contents;

	unsigned int depth;

	JSON_WRITE_TYPE previous_token;
	JSON_WRITE_TYPE *hierarchy;
}JSONWrite;

/**
 * @brief Create a new JSONWrite context
 * @return A freshly initialized JSONWrite context
*/
JSONWrite JSONWriteNew();

/**
 * @brief Append an element to the JSONWrite context
 * @param json The json context to operate on
 * @param type The type of element to be appended
 * @param element_name (optional when writing a string, int, float, or bool)
 * @param value Must correspond to 'type', must be NULL for array and object, cannot be NULL otherwise
*/
void JSONWriteElement(JSONWrite *json, JSON_WRITE_TYPE type, const char *element_name, const void *value);

/**
 * @brief 
 * @param json The json context to operate on
*/
void JSONWriteIndent(JSONWrite *json);

/**
 * @brief 
 * @param json The json context to operate on
*/
void JSONWriteOutdent(JSONWrite *json);

/**
 * @brief Saves the accumulated contents of the JSONWrite context to a file, doing this sends you to the root of the context
 * @param json The json context to operate on
 * @param path The directory and filename of the destination json file
*/
void JSONWriteToFile(JSONWrite *json, char *path);

/**
 * @brief Closes the specified JSONWrite context
 * @param json The json context to operate on
*/
void JSONWriteFree(JSONWrite *json);

#endif