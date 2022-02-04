#include "global.h"
#include "debug.h"
#include "gl_context.h"
#include "renderer/renderer.h"
#include "vectorlib.h"

#include "gl_utils.h"

// Active opengl references
unsigned int current_vao;
unsigned int bound_textures[16];
unsigned int current_texture_unit;

// Texture undefined_texture;

void InitGLUtils(){
	// undefined_texture = TextureOpen("../images/undefined.png");
}

void CheckGLErrors(const char *file, int line){
    unsigned int error_code;
    while((error_code = glGetError()) != GL_NO_ERROR){
        char error_string[32];
        switch(error_code){
            case 1280:
                strcpy(error_string, "INVALID_ENUM");
                break;
            case 1281:
                strcpy(error_string, "INVALID_VALUE");
                break;
            case 1282:
                strcpy(error_string, "INVALID_OPERATION");
                break;
            case 1283:
                strcpy(error_string, "STACK_OVERFLOW");
                break;
            case 1284:
                strcpy(error_string, "STACK_UNDERFLOW");
                break;
            case 1285:
                strcpy(error_string, "OUT_OF_MEMORY");
                break;
            case 1286:
                strcpy(error_string, "INVALID_FRAMEBUFFER_OPERATION");
                break;
			default:
				break;
        }
        DebugLog(D_ERR, "Opengl error '%s' ('%s': %d)", error_string, file, line);
        printf("Opengl error '%s' ('%s': %d)", error_string, file, line);
    }
}





void SetVAO(unsigned int vao){
	if(current_vao != vao){
		glBindVertexArray(vao);
		current_vao = vao;
	}
}