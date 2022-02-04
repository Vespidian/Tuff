mak@shader vertex
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
uniform mat4 orthographic_projection;

void main() {
	mask_src_f = mask_src_a;
	top_src_f = top_src_a;
	bottom_src_f = bottom_src_a;
	color_f = color_a;

	int quad_vert = gl_VertexID % 4;
	texture_coordinate_v = vec2(tex_coordinates[quad_vert][0], tex_coordinates[quad_vert][1]);

	gl_Position = orthographic_projection * model_a * vec4(pos_a, 1);
}

@shader fragment
#version 130

flat in vec4 color_f;
flat in vec4 top_src_f;
flat in vec4 bottom_src_f;
flat in vec4 mask_src_f;
in vec2 texture_coordinate_v;

out vec4 FragColor;

uniform sampler2D top_texture_s;
uniform sampler2D bottom_texture_s;
uniform sampler2D mask_texture_s;

uniform vec2 mouse_pos;

void main() {
	vec2 texture_size = textureSize(mask_texture_s, 0);

	vec2 texture_coord_scaled = texture_coordinate_v;
	texture_coord_scaled *= mask_src_f.zw / texture_size;
	texture_coord_scaled += mask_src_f.xy / texture_size;

	if(texture(mask_texture_s, texture_coord_scaled).a == 1){
		texture_size = textureSize(top_texture_s, 0);
		texture_coord_scaled = texture_coordinate_v;
		texture_coord_scaled *= top_src_f.zw / texture_size;
		texture_coord_scaled += top_src_f.xy / texture_size;
		FragColor = texture(top_texture_s, texture_coord_scaled);
	}else{
		texture_size = textureSize(bottom_texture_s, 0);
		texture_coord_scaled = texture_coordinate_v;
		texture_coord_scaled *= bottom_src_f.zw / texture_size;
		texture_coord_scaled += bottom_src_f.xy / texture_size;
		FragColor = texture(bottom_texture_s, texture_coord_scaled);
		FragColor *= vec4(0.5, 0.5, 0.5, 1);
	}

	FragColor *= color_f;
}