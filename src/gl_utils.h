#ifndef GL_UTILS_H_
#define GL_UTILS_H_

void InitGLUtils();

#include "vectorlib.h"
#include <cglm/cglm.h>

/**
 *  Active VAO
 */
extern unsigned int current_vao;

/**
 *  Ordered array of all bound textures
 */
extern unsigned int bound_textures[16];

/**
 *  Bound texture unit
 */
extern unsigned int current_texture_unit;

void CheckGLErrors(const char *file, int line);

void SetVAO(unsigned int vao);

#endif