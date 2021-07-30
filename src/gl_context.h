#ifndef GL_CONTEXT_H_
#define GL_CONTEXT_H_

#define DEBUG_BUILD

#ifdef DEBUG_BUILD
#define GLCall CheckGLErrors(__FILE__, __LINE__);
#else
#define GLCall
#endif

extern SDL_GLContext gl_context;
int InitGL();


//TMP
void RenderGL();

extern unsigned int default_shader;

extern int z_depth;

//tmp
#include "gl_utils.h"


#endif