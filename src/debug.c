#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "debug.h"

static bool initialized = false;

FILE *log_file;
int log_complexity = DL_VERBOSE;

void InitDebug(){
	if(!initialized){
		initialized = true;
		log_file = fopen("../log.txt", "a");
		fprintf(log_file, "\n---------------\nSEPARATOR\n---------------\n\n");
	}
}

void DebugLog(int type, const char *format, ...){
	if(!initialized){
		InitDebug();
	}
	if(format != NULL){
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

			//Insert timestamp to log
			if(type <= log_complexity){
				time_t rawtime;
				struct tm *currentTime;
				time(&rawtime);
				currentTime = localtime(&rawtime);
				fprintf(log_file, "%d-%d-%d %d:%d:%d: ", 1900 + currentTime->tm_year, currentTime->tm_mon,
				currentTime->tm_mday, currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);
			}
			if(formatted_text[strlen(formatted_text) - 1] == '\n'){
				formatted_text[strlen(formatted_text) - 1] = 0;
			}
			
			//Insert corresponding debug log type depending on defined log complexity
			if(type <= log_complexity){
				switch(type){
					//Brief
					case D_ACT:
						fprintf(log_file, "[ACTION]: ");
						break;
					case D_WARN:
						fprintf(log_file, "[WARNING]: ");
						break;
					case D_ERR:
						fprintf(log_file, "[ERROR]: ");
						break;
					case D_SCRIPT_ERR:
						fprintf(log_file, "[LUA_ERROR]: ");
						break;
					case D_SCRIPT_ACT:
						fprintf(log_file, "[LUA_ACTION]: ");
						break;

					//Verbose
					case D_VERBOSE_ACT:
						fprintf(log_file, "[ACTION]: ");
						break;
					case D_VERBOSE_WARN:
						fprintf(log_file, "[WARNING]: ");
						break;
					case D_VERBOSE_ERR:
						fprintf(log_file, "[ERROR]: ");
						break;

					default:
						break;
				}
				fprintf(log_file, "%s\n", formatted_text);
			}
			
			//Free the formatted string
			free(formatted_text);
			formatted_text = NULL;
			//Flush the file buffer to the file
			fflush(log_file);
		}
		va_end(va_format);
	}
}

void QuitDebug(){
	if(initialized){
		initialized = false;
		fclose(log_file);
	}
}