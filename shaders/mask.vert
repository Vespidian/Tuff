#version 130
#extension GL_ARB_explicit_attrib_location : enable
layout (location = 0) in vec3 pos_a;

layout(location = 1) in mat4 model_a;
layout(location = 5) in vec4 color_a;
layout(location = 6) in vec4 top_src_a;
layout(location = 7) in vec4 bottom_src_a;
layout(location = 8) in vec4 mask_src_a;

flat out vec4 top_src_f;
flat out vec4 bottom_src_f;
flat out vec4 mask_src_f;
flat out vec4 color_f;
out vec2 texture_coordinate_v;

uniform mat4 tex_coordinates;
uniform mat4 projection_matrix;

void main() {
	mask_src_f = mask_src_a;
	top_src_f = top_src_a;
	bottom_src_f = bottom_src_a;
	color_f = color_a;

	int quad_vert = gl_VertexID % 4;
	texture_coordinate_v = vec2(tex_coordinates[quad_vert][0], tex_coordinates[quad_vert][1]);

	gl_Position = projection_matrix * model_a * vec4(pos_a, 1);
}