#version 130
#extension GL_ARB_explicit_attrib_location : enable

layout (location = 0) in vec3 pos_a;
layout (location = 1) in vec3 texture_a;
layout (location = 2) in vec3 normal_a;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 vert_v;
out vec3 normal_v;
out vec3 texture_v;

void main(){
	// vert_v = (projection * view * model * vec4(pos_a, 1)).xyz;
	vert_v = pos_a;
	normal_v = normal_a;
	texture_v = texture_a;
	gl_Position = projection * view * model * vec4(pos_a, 1);
	// gl_Position = projection * view * model * vec4(normal_a, 1);
	// gl_Position = projection * view * model * vec4(texture_a, 1);
}

// #version 130
// #extension GL_ARB_explicit_attrib_location : enable
// layout (location = 0) in vec3 pos_a;

// // Instanced
// layout (location = 1) in mat4 model_a;
// layout (location = 5) in vec4 color_a;
// layout (location = 6) in vec4 texture_src_a;

// uniform mat4 tex_coordinates;
// uniform mat4 projection;


// out vec2 tex_coordinate_v;
// out vec4 color_v;
// flat out vec4 texture_src_f;


// void main(){
// 	gl_Position = projection * model_a * vec4(pos_a.xyz, 1);
    
//     int quadVert = gl_VertexID % 4;
//     tex_coordinate_v = vec2(tex_coordinates[quadVert][0], tex_coordinates[quadVert][1]);
//     color_v = color_a;
// 	texture_src_f = texture_src_a;
// }