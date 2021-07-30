@shader vertex
#version 140
#extension GL_ARB_explicit_attrib_location : enable

layout (location = 0) in vec3 pos_a;
layout (location = 1) in vec3 color_a;

layout (std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};

uniform vec3 grid_pos_u;

out vec3 pos_v;
out vec3 color_v;
out vec3 grid_pos_v;

void main(){
	gl_Position = projection * view * vec4(pos_a, 1);
	// gl_Position = vec4(pos_a, 1);
	pos_v = pos_a;
	grid_pos_v = grid_pos_u;

	if(pos_a.z == 0){
		color_v = vec3(1, 0.2, 0.322);
	}else if(pos_a.x == 0){
		color_v = vec3(0.157, 0.565, 1);
	}else{
		color_v = color_a;
	}
}


@shader fragment
#version 130
out vec4 FragColor;

in vec3 pos_v;
in vec3 color_v;
in vec3 grid_pos_v;

void main(){
	FragColor = vec4(color_v, 1);
	// FragColor *= 4 - distance(vec3(0), pos_v);
}
// #version 140
// #extension GL_ARB_explicit_attrib_location : enable

// layout (location = 0) in vec3 pos_a;

// layout (std140) uniform Matrices{
// 	mat4 projection;
// 	mat4 view;
// };

// out mat4 projection_frag;
// out mat4 view_frag;

// uniform mat4 model;

// out vec3 far_point;
// out vec3 near_point;

// out vec3 position_v;

// vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) {
//     mat4 viewInv = inverse(view);
//     mat4 projInv = inverse(projection);
//     vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
//     return unprojectedPoint.xyz / unprojectedPoint.w;
// }

// void main(){
// 	gl_Position = projection * view * vec4(pos_a * 25, 1);

// 	near_point = UnprojectPoint(pos_a.x, pos_a.y, 0.0, view, projection).xyz; // unprojecting on the near plane
//     far_point = UnprojectPoint(pos_a.x, pos_a.y, 1.0, view, projection).xyz; // unprojecting on the far plane
    
// 	projection_frag = projection;
// 	view_frag = view;

// 	position_v = pos_a;
// }


// #version 130
// out vec4 FragColor;

// in float view_depth;

// in vec3 far_point;
// in vec3 near_point;

// in mat4 projection_frag;
// in mat4 view_frag;

// in vec3 position_v;

// uniform sampler2D texture_0;

// float gridTextureGradBox( in vec2 p, in vec2 ddx, in vec2 ddy, float grid_ratio )
// {
// 	// filter kernel
//     vec2 w = max(abs(ddx), abs(ddy)) + 0.01;

// 	// analytic (box) filtering
//     vec2 a = p + 0.5*w;                        
//     vec2 b = p - 0.5*w;           
//     vec2 i = (floor(a)+min(fract(a)*grid_ratio,1.0)-
//               floor(b)-min(fract(b)*grid_ratio,1.0))/(grid_ratio*w);
//     //pattern
//     return (1.0-i.x)*(1.0-i.y);
// }

// void main(){
// 	vec4 tmp = ((projection_frag * view_frag * vec4(position_v, 1)) - 1.5);

// 	// vec2 uv = position_v.xz;
// 	// float size = 1.0/8.0;   // size of the tile
// 	// // float edge = size/32.0; // size of the edge
// 	// // float edge = size/(gl_FragCoord.w * 32); // size of the edge
// 	// float edge = size/((1 - view_depth) * 16); // size of the edge
// 	// // float edge = size/((tmp.z) * 32); // size of the edge
// 	// uv = (mod(uv + edge / 2, size) - mod(uv - edge / 2, size) - edge) * 1.0/size;
// 	// FragColor = vec4(length(uv) * 0.8);
// 	// // 0.9 controls the brighness of the face of the tile
// 	// // 0.5 controls the darkness of the edge

// 	// FragColor *= 1 - distance(vec3(0), position_v);

// 	// FragColor = vec4(vec3(mod((position_v.z + 1) / 2, 0.1)), 1);
// 	// FragColor = vec4(vec3(ceil(fract((position_v.z + 1) * 4))), 1);
// 	// coordinates


// 	// float grid_ratio = 10;
//     // vec2 i = step( fract((tmp.xy + 1) * 10), vec2(1.0/grid_ratio) );
//     // //pattern
// 	// FragColor = vec4(vec3((1.0-i.x)*(1.0-i.y)), 1);


// 	// FragColor = vec4(vec3(step(fract(position_v.x * 2)*fract(position_v.z * 2), 0.1)), 1);
// 	// FragColor = vec4(vec3(mod(position_v.x, 0.5)), 1);


// 	// vec2 ddx = dFdx(position_v.xz);
// 	// vec2 ddy = dFdy(position_v.xz);
// 	// // filter kernel
//     // vec2 w = max(abs(ddx), abs(ddy)) + 0.01;

// 	// // analytic (box) filtering
//     // vec2 a = position_v.xz + w;
//     // vec2 b = position_v.xz - w;

// 	// float edge_width = 0.05;
// 	// vec2 top_right = vec2(step(vec2(edge_width / 2), fract((a) * 100)));
// 	// vec2 bottom_left = vec2(step(vec2(edge_width / 2) * w, fract((b) * 100)));
// 	// // vec2 top_right = vec2(step(vec2(edge_width / 2), fract((position_v.xz) * 100)));
// 	// // vec2 bottom_left = vec2(step(vec2(edge_width / 2), fract((1 - position_v.xz) * 100)));
// 	// FragColor = vec4(1 - top_right.x * top_right.y * bottom_left.x * bottom_left.y);


// 	// FragColor = texture(texture_0, fract(position_v.xz * 25));
// 	// FragColor.a *= 1 - distance(position_v.xz, vec2(0)) * 5;

// 	// float grid_spacing = 10;
// 	// float grid_thickness = 0.01;
// 	// // mod(fract(position_v.xz / grid_spacing), grid_thickness)
// 	// FragColor = vec4(vec3(fract(position_v.x / grid_spacing)), 1);

// 	// //most promising
// 	float num_cells = 200;
// 	float grid_ratio = 64;
// 	float v1 = gridTextureGradBox(position_v.xz * num_cells / 2, dFdx(position_v.xz * num_cells / 2), dFdy(position_v.xz * num_cells / 2), grid_ratio);
// 	float v2 = gridTextureGradBox(1 - position_v.xz * num_cells / 2, dFdx(1 - position_v.xz * num_cells / 2), dFdy(1 - position_v.xz * num_cells / 2), grid_ratio);
// 	FragColor = vec4(1 - v1 * v2);
// 	// FragColor.a *= 1 - distance(position_v.xz, vec2(0)) * 5;
// 	// FragColor = vec4(1 - gridTextureGradBox(1 - position_v.xz * 4, dFdx(1 - position_v.xz * 4), dFdy(1 - position_v.xz * 4)));
// 	FragColor.rgb *= 2;
// 	if(FragColor.a < 0.01){
// 		discard;
// 	}

// 	// float t = -near_point.y / (far_point.y - near_point.y);
// 	// vec3 fragPos3D = near_point + t * (far_point - near_point);

// 	// vec4 tmp = (projection_frag * view_frag * vec4(fragPos3D.xyz, 1));
// 	// gl_FragDepth = (tmp.z / tmp.w);


// 	// FragColor = vec4(1, 1, 1, t > 0 ? 0.8 : 0);
// 	// FragColor = vec4(fragPos3D.zzz, t > 0 ? 0.8 : 0);
// 	// FragColor = vec4(vec3(tmp.z / tmp.w) / 10, t > 0 ? 0.8 : 0);
// 	// FragColor = vec4(tmp.zzz / tmp.www, 1);
// 	// FragColor = vec4(1, 1, 1, 1);

// 	// FragColor = vec4((tmp.zzz), 1);
// }