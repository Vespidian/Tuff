#version 130

out vec4 FragColor;

in vec3 vert_v;
in vec3 normal_v;
in vec3 texture_v;

uniform sampler2D tex;


void main(){
	// float grid_spacing = 0.1;
	// FragColor = vec4(vert_v.xyz / vec3(2.5), 1);
	// FragColor = vec4(abs(normal_v.xyz) * (1 - vert_v.zzz / vec3(5)), 1);
	// FragColor = vec4(abs(normal_v.xyz), 1);
	// FragColor = vec4(texture_v.xyz, 1);
	// FragColor = texture(tex, texture_v.xy) * vec4((1 - vert_v.zzz / vec3(3.5)), 1);

	FragColor = vec4(normal_v.xyz, 1);
	// FragColor = vec4(0, 1, 0, 1);
	// if(fract(texture_v.x / 0.001f) < 0.025f || fract(texture_v.y / 0.001f) < 0.025f)
    //     FragColor = vec4(1);
    // else
    //     FragColor = vec4(0);
}

// vec4 params = {};

// float grid(vec2 st, float res)
// {
//   vec2 grid = fract(st*res);
//   return (step(res,grid.x) * step(res,grid.y));
// }
 
// void main()
// {
//   vec2 grid_uv = texture_v.xy * params.x; // scale
//   float x = grid(grid_uv, params.y); // resolution
//   gl_FragColor.rgb = vec3(0.5) * x;  
//   gl_FragColor.a = 1.0;
// }