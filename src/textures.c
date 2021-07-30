#include "global.h"
#include "debug.h"
#include "renderer/renderer.h"

#include "textures.h"

TextureObject undefined_texture;

TextureObject *texture_stack;

// Number of textures currently loaded into the 'texture_stack'
unsigned int num_textures = 0;

void InitTextures(){
    undefined_texture = LoadTexture("../images/undefined.png", GL_RGBA);
    DebugLog(D_ACT, "Initialized texture subsystem");
}

TextureObject *LoadTextureToStack(const char *path, int format){
	// Expand 'texture_stack' to fit new texture
    texture_stack = realloc(texture_stack, sizeof(TextureObject) * (num_textures + 1));
    
	// Load the new texture
	texture_stack[num_textures] = LoadTexture(path, format);

    return &texture_stack[num_textures++];
}

TextureObject *FindTexture(unsigned int texture){
    for(int i = 0; i < num_textures; i++){
        if(texture_stack[i].gl_tex == texture){
            return &texture_stack[i];
        }
    }
    return &undefined_texture;
}