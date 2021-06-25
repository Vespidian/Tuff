#version 130

out vec4 FragColor;

in vec3 vert_v;
in vec3 normal_v;
in vec3 texture_v;

in vec3 frag_pos_v;

uniform sampler2D tex;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 view_position;

void main(){
	vec3 ambient = vec3(0.25);
	float specular_strength = 0.5;

	// Calculate diffuse
	vec3 light_dir = normalize(light_pos - frag_pos_v);
	float difference = max(dot(normal_v, light_dir), 0);
	vec3 diffuse = vec3(difference * light_color);

	// Calculate specular
	vec3 view_dir = normalize(view_position - frag_pos_v);
	vec3 reflect_dir = reflect(-light_dir, normal_v);
	float spec = pow(max(dot(view_dir, reflect_dir), 0), 64);
	// vec3 specular = specular_strength * spec * light_color;
	vec3 specular = vec3(specular_strength * spec);
	
	// FragColor = texture(tex, texture_v.xy);
	FragColor = vec4(abs(normal_v.zzz), 1);
	// FragColor *= vec4(diffuse, 1);
	FragColor *= vec4(diffuse + specular + ambient, 1);
	// FragColor = vec4(light_pos, 1);

}