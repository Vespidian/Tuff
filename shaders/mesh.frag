#version 130

out vec4 FragColor;

in vec3 vert_v;
in vec3 normal_v;
in vec3 texture_v;

uniform sampler2D tex;

void main(){

	FragColor = texture(tex, texture_v.xy);

}