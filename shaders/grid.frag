#version 130

out vec4 FragColor;

in vec3 vert_v;
in vec3 normal_v;
in vec3 texture_v;

uniform sampler2D tex;

void main(){
	float grid_spacing = 0.1;
	// FragColor = vec4(vert_v.xyz / vec3(2.5), 1);
	// FragColor = vec4(abs(normal_v.xyz) * (1 - vert_v.zzz / vec3(5)), 1);
	// FragColor = vec4(abs(normal_v.xyz), 1);
	// FragColor = vec4(texture_v.xyz, 1);
	// FragColor = texture(tex, texture_v.xy) * vec4((1 - vert_v.zzz / vec3(3.5)), 1);

	// FragColor = vec4(normal_v.xyz, 1);
	if(fract(texture_v.x / 0.005f) < 0.025f || fract(texture_v.y / 0.005f) < 0.025f)
        FragColor = vec4(1);
    else
        FragColor = vec4(0, 0, 0, 0.5);
		// discard;
	FragColor.a *= 1 - distance(vert_v.xyz, vec3(0)) * 10;

	if(distance(vert_v.x, 0) * 100 < 0.025f){
		FragColor = vec4(1, 0, 0, 1);
	}
	if(distance(vert_v.z, 0) * 100 < 0.025f){
		FragColor = vec4(0, 0, 1, 1);
	}

	if(distance(vert_v.xyz, vec3(0)) * 10 < 0.01){
		FragColor = vec4(0, 1, 0, 1);
	}
	// FragColor.a =  vert_v.x;
}