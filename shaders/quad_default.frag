#version 130

out vec4 FragColor;

in vec2 tex_coordinate_v;
in vec4 color_v;
flat in vec4 texture_src_f;

uniform sampler2D src_texture;

void main(){
    vec2 tex_coord = tex_coordinate_v;
    vec2 texture_size = textureSize(src_texture, 0);

    tex_coord *= texture_src_f.zw / texture_size;
    tex_coord += texture_src_f.xy / texture_size;

    FragColor = texture(src_texture, tex_coord) * color_v;
	// if(distance(vec2(0.5), tex_coordinate_v.xy)*3 > 1.3){
	if(FragColor.a < 0.01){
		discard;
	}
}