#ifndef ENGINE_H_
#define ENGINE_H_

/**
 *  Orthographic projection matrix for use with ui rendering
 */
extern mat4 orthographic_projection;

/**
 *  Perspective projection matrix for rendering of 3D objects
 */
extern mat4 perspective_projection;

/**
 *  View matrix - changes depending on orientation and position of camera
 */
extern mat4 view_matrix;

/**
 * Uniform buffer object - stores a few globally changing shader uniforms (time, matrices)
*/
extern unsigned int uniform_buffer;

/**
 * 
*/
extern unsigned int engine_time;

#endif