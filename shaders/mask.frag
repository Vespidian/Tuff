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