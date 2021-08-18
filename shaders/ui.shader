@shader vertex
#version 130
#extension GL_ARB_explicit_attrib_location : enable
layout (location = 0) in vec3 pos_a;

// Instanced
layout (location = 1) in vec4 border_a;
layout (location = 2) in vec4 radius_a;
layout (location = 3) in vec4 color_a;
layout (location = 4) in vec4 border_color_a;


layout (std140) uniform Matrices{
	mat4 projection;
	mat4 view;
	int time;
};

out vec4 border_v;
out vec4 radius_v;
out vec4 color_v;
out vec4 border_color_v;

void main(){

	border_v = border_a;
	radius_v = radius_a;
	color_v = color_a;
	border_color_v = border_color_a;

	gl_Position = projection * model * vec4(pos_a, 1);

}

@shader fragment
#version 130

out vec4 FragColor;

in vec4 border_v;
in vec4 radius_v;
in vec4 color_v;
in vec4 border_color_v;

uniform sampler2D tex;

void main(){

	FragColor = vec4(1);

}