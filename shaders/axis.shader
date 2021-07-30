@shader vertex
#version 140
#extension GL_ARB_explicit_attrib_location : enable

layout (location = 0) in vec3 pos_a;
layout (location = 1) in vec3 color_a;

layout (std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};

out vec3 color_v;

uniform mat4 model;

void main(){
	color_v = color_a;
	gl_Position = projection * view * vec4(pos_a, 1);
}

@shader fragment
#version 130

out vec4 FragColor;

in vec3 color_v;

void main(){

	FragColor = vec4(color_v, 1);
	// FragColor = vec4(1, 0, 0, 1);

}