@shader vertex
#version 140
#extension GL_ARB_uniform_buffer_object : enable
#extension GL_ARB_explicit_attrib_location : enable

layout (location = 0) in vec3 pos_a;
layout (location = 1) in vec3 normal_a;
layout (location = 2) in vec2 texture_a;
layout (location = 3) in vec3 tangent_a;

layout (std140) uniform ShaderGlobals{
	mat4 projection_persp;
	mat4 projection_ortho;
	mat4 view;
	float time;
};

uniform mat4 model;

out vec3 vert_v;
out vec3 normal_v;
out vec2 texture_v;
out mat3 TBN_v;

// Used by light shader
out vec3 frag_pos_v;
uniform float value;
float PHI = 1.61803398874989484820459;  // Φ = Golden Ratio  

float gold_noise(in vec2 xy, in float seed){
	return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}

void main(){
	vec3 T = normalize(vec3(model * vec4(tangent_a, 0)));
	vec3 B = normalize(vec3(model * vec4(cross(normal_a, tangent_a), 0)));
	vec3 N = normalize(vec3(model * vec4(normal_a, 0)));
	mat3 TBN = mat3(T, B, N);
	// vert_v = (projection * view * model * vec4(pos_a, 1)).xyz;
	vert_v = pos_a;
	normal_v = normal_a;
	texture_v = texture_a;
	// float value = time * gold_noise(pos_a.xz, 502);
	// gl_Position = projection * view * model * vec4(pos_a + vec3(sin(value / 300.0) / 2), 1);
	gl_Position = projection_persp * view * model * vec4(pos_a, 1);
	// gl_Position = projection * view * model * vec4(normal_a, 1);
	// gl_Position = projection * view * model * vec4(texture_a, 1);
	frag_pos_v = vec3(model * vec4(pos_a, 1));
	gl_PointSize = gl_Position.z;

}

@shader fragment
#version 130

out vec4 FragColor;

in vec3 vert_v;
in vec3 normal_v;
in vec2 texture_v;
in mat3 TBN_v;
uniform float value;

in vec3 frag_pos_v;

uniform sampler2D tex;
uniform sampler2D normal_map;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 view_position;

uniform float normal_map_intensity;

void main(){

	// vec3 new_normal = normalize(normal_v + (texture(normal_map, texture_v.xy)).rgb);
	// new_normal *= 2.0 - 1.0;
	// new_normal = normalize(TBN_v * new_normal);
	vec3 new_normal = normalize(normal_v);
	// vec3 new_normal = normalize((texture(normal_map, texture_v.xy)).rgb);

	vec3 ambient = vec3(0.25);
	vec3 sun_dir = normalize(radians(vec3(20, 45, 0)));
	ambient += (dot(sun_dir, normalize(new_normal)) + 1) / 4;
	// if(dot(sun_dir, normalize(normal_v)) <= 0.0){
	// 	ambient = vec3(max(dot(sun_dir, normalize(normal_v)), 0) + 0.25);
	// }
	// vec3 sun = vec3(max(dot(sun_dir, normalize(normal_v)), 0));
	// float specular_strength = 0.5;
	float specular_strength = 1;

	// Calculate diffuse
	vec3 light_dir = normalize(light_pos - frag_pos_v);
	float difference = max(dot(new_normal, light_dir), 0);
	vec3 diffuse = vec3(difference * light_color);

	// Calculate specular
	vec3 view_dir = normalize(view_position - frag_pos_v);
	vec3 reflect_dir = reflect(-light_dir, new_normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0), 32);
	vec3 specular = specular_strength * spec * light_color;

	//tmp
	reflect_dir = reflect(-sun_dir, new_normal);
	spec = pow(max(dot(view_dir, reflect_dir), 0), 32);
	specular += vec3(specular_strength * spec);
	// vec3 specular = specular_strength * spec * light_color;
	// vec3 specular = vec3(specular_strength * spec);
	
	// FragColor = texture(tex, vec2(texture_v.x, 1 - texture_v.y));
	// FragColor = vec4(abs(vert_v.zzz), 1);
	// FragColor = vec4(abs(vert_v.zzz), 1) + texture(tex, texture_v.xy);
	FragColor = vec4(vec3(0.75), 1);
	// FragColor *= vec4(diffuse, 1);
	FragColor *= vec4(diffuse + specular + ambient, 1);
	// FragColor *= vec4(diffuse + ambient, 1);
	// FragColor = vec4(diffuse + specular + ambient, 1);
	// FragColor *= vec4(ambient + specular, 1);
	// FragColor = vec4(light_pos, 1);
	// FragColor = texture(normal_map, texture_v.xy);


}