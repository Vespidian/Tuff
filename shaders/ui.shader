@shader vertex
#version 140
#extension GL_ARB_explicit_attrib_location : enable
layout (location = 0) in vec3 pos_a;

// Instanced
layout (location = 1) in mat4 model_a;
layout (location = 5) in vec4 border_a;
layout (location = 6) in vec4 radius_a;
layout (location = 7) in vec4 color_a;
layout (location = 8) in vec4 border_color_a;

layout (std140) uniform ShaderGlobals{
	mat4 projection_persp;
	mat4 projection_ortho;
	mat4 view;
	float time;
};

uniform mat4 tex_coordinates;

out vec4 border_v;
out vec4 radius_v;
out vec4 color_v;
out vec4 border_color_v;
out vec2 tex_coordinate_v;
out float time_f;
flat out vec2 pixel_scale_f;

void main(){

	time_f = time;

    int quadVert = gl_VertexID % 4;
    tex_coordinate_v = vec2(tex_coordinates[quadVert][0], tex_coordinates[quadVert][1]);
	
	border_v = border_a;
	radius_v = radius_a;
	color_v = color_a;
	border_color_v = border_color_a;

	pixel_scale_f = vec2(model_a[0][0], model_a[1][1]);

	gl_Position = projection_ortho * model_a * vec4(pos_a, 1);

}

@shader fragment
#version 130

out vec4 FragColor;

in vec4 border_v;
in vec4 radius_v;
in vec4 color_v;
in vec4 border_color_v;
in vec2 tex_coordinate_v;
in float time_f;
flat in vec2 pixel_scale_f;


uniform sampler2D tex;
// uniform float time;

const float PI = 3.141592653589793;

/*
float roundedBoxSDF(vec2 CenterPosition, vec2 Size, float Radius) {
    return length(max(abs(CenterPosition)-Size+Radius,0.0))-Radius;
}*/

float roundedBoxSDF(vec2 CenterPosition, vec2 Size, vec4 Radius) {
    Radius.xy = (CenterPosition.x>0.0) ? Radius.xy : Radius.zw;
    Radius.x  = (CenterPosition.y>0.0) ? Radius.x  : Radius.y;
    
    vec2 q = abs(CenterPosition)-Size+Radius.x;
    return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - Radius.x;
}

float circle(vec2 uv, float radius, vec2 position){
	uv -= position;

	float d = length(uv) - radius;

	return d > 0 ? 0 : 1;
}

float rectangle(vec2 uv, vec2 scale, vec2 position) {
    vec2 shaper = step(position, uv);
    shaper *= step(uv, position + scale);

    return shaper.x * shaper.y;
}

float CircleSweep(vec2 uv, float progress, vec2 position){
	// Progress is a value between 0 and 1. 0.0 being empty and 1.0 full
	uv -= position;
	float angle = atan(uv.x, uv.y) / PI;
	if (angle < 2 * progress - 1) {    
		return 1;
	}
	return 0;
}

vec2 CalculateOffset(float radius, vec2 element_scale){
	return vec2((radius * 2) / min(element_scale.x, element_scale.y), (radius * 2) / min(element_scale.x, element_scale.y));
}

void main(){
	vec2 tex_coord = (tex_coordinate_v) * 2 - 1;

	// vec4 circleColor = vec4(color_v.xyz, 1);
	// // Progress is a value between 0 and 1. 0 being empty and 1 full
	// float progress = sin(time_f * 3) * 0.5 + 0.5;
    // FragColor = vec4(0);
	// float angle = atan(tex_coord.x, tex_coord.y) / PI;
	// if (angle < 2*progress - 1) {
	// 	// Uncomment this to get a gradient
	// 	// FragColor = vec4((angle * 0.5 + 0.5)*circleColor);    
	// 	FragColor = vec4(circleColor);    
	// }
	


	// float border = (border_v.x * 2) / min(pixel_scale_f.x, pixel_scale_f.y);
	// vec4 border = vec4(10, 2, 0, 0);
	vec4 border = (border_v * 2) / min(pixel_scale_f.x, pixel_scale_f.y);
	// border = (border * 2) / min(pixel_scale_f.x, pixel_scale_f.y);
	// vec2 rxoffset = ((border.x > border.w) ? vec2(border.w - border.x, 0) : vec2(0, border.x - border.w));
	// vec2 ryoffset = ((border.x > border.y) ? vec2(border.y - border.x, 0) : vec2(0, border.x - border.y));
	// vec2 rzoffset = ((border.y > border.z) ? vec2(border.z - border.y, 0) : vec2(0, border.y - border.z));
	// vec2 rwoffset = ((border.z > border.w) ? vec2(border.w - border.z, 0) : vec2(0, border.z - border.w));

	vec4 radiuses = radius_v;
	radiuses = (radiuses * 2) / vec4(min(pixel_scale_f.x, pixel_scale_f.y));

	vec2 aspect_ratio = vec2(pixel_scale_f.x / pixel_scale_f.y, 1);
	if(pixel_scale_f.y > pixel_scale_f.x){
		aspect_ratio = vec2(1, pixel_scale_f.y / pixel_scale_f.x);
		tex_coord.y *= pixel_scale_f.y / pixel_scale_f.x;
	}else{
		tex_coord.x *= pixel_scale_f.x / pixel_scale_f.y;
	}

	vec2 top_left = vec2(radiuses.x - aspect_ratio.x, aspect_ratio.y - radiuses.x);
	vec2 top_right = vec2(aspect_ratio.x - radiuses.y, aspect_ratio.y - radiuses.y);
	vec2 bottom_right = vec2(aspect_ratio.x - radiuses.z, radiuses.z - aspect_ratio.y);
	vec2 bottom_left = vec2(radiuses.w - aspect_ratio.x, radiuses.w - aspect_ratio.y);

	vec4 pixel_type = vec4(0);

	// Border radius
	pixel_type = bool(circle(tex_coord, radiuses.x, top_left)) ? vec4(1, 0, 0, 0) : pixel_type; // Top Left
	pixel_type = bool(circle(tex_coord, radiuses.y, top_right)) ? vec4(1, 0, 0, 0) : pixel_type; // Top Right
	pixel_type = bool(circle(tex_coord, radiuses.z, bottom_right)) ? vec4(1, 0, 0, 0) : pixel_type; // Bottom Right
	pixel_type = bool(circle(tex_coord, radiuses.w, bottom_left)) ? vec4(1, 0, 0, 0) : pixel_type; // Bottom Left

	// Inner radius
	// pixel_type = bool(circle(tex_coord, radiuses.x - border, top_left)) ? vec4(0, 1, 0, 1) : pixel_type; // Top Left
	// pixel_type = bool(circle(tex_coord, radiuses.y - border, top_right)) ? vec4(0, 1, 0, 1) : pixel_type; // Top Right
	// pixel_type = bool(circle(tex_coord, radiuses.z - border, bottom_right)) ? vec4(0, 1, 0, 1) : pixel_type; // Bottom Right
	// pixel_type = bool(circle(tex_coord, radiuses.w - border, bottom_left)) ? vec4(0, 1, 0, 1) : pixel_type; // Bottom Left
	pixel_type = bool(circle(tex_coord, radiuses.x - max(border.x, border.w), top_left)) ? vec4(0, 1, 0, 1) : pixel_type; // Top Left
	pixel_type = bool(circle(tex_coord, radiuses.y - max(border.x, border.y), top_right)) ? vec4(0, 1, 0, 1) : pixel_type; // Top Right
	pixel_type = bool(circle(tex_coord, radiuses.z - max(border.y, border.z), bottom_right)) ? vec4(0, 1, 0, 1) : pixel_type; // Bottom Right
	pixel_type = bool(circle(tex_coord, radiuses.w - max(border.z, border.w), bottom_left)) ? vec4(0, 1, 0, 1) : pixel_type; // Bottom Left

	// Inner area
	pixel_type = bool(rectangle(tex_coord, vec2(aspect_ratio.x * 2 - radiuses.x - radiuses.y, aspect_ratio.y - aspect_ratio.y + max(radiuses.x, radiuses.y)), vec2(radiuses.x - aspect_ratio.x, aspect_ratio.y - max(radiuses.x, radiuses.y)))) ? vec4(0, 0, 0, 1) : pixel_type; // Top
	pixel_type = bool(rectangle(tex_coord, vec2(aspect_ratio.x * 2 - radiuses.w - radiuses.z, aspect_ratio.y + aspect_ratio.y - max(radiuses.x, radiuses.y)), vec2(radiuses.w - aspect_ratio.x, -aspect_ratio.y))) ? vec4(0, 0, 0, 1) : pixel_type; // Bottom
	pixel_type = bool(rectangle(tex_coord, vec2(aspect_ratio.x - aspect_ratio.x + max(radiuses.y, radiuses.z), aspect_ratio.y * 2 - radiuses.y - radiuses.z), vec2(aspect_ratio.x - max(radiuses.y, radiuses.z), radiuses.z - aspect_ratio.y))) ? vec4(0, 0, 0, 1) : pixel_type; // Right
	pixel_type = bool(rectangle(tex_coord, vec2(aspect_ratio.x + aspect_ratio.x - max(radiuses.y, radiuses.z), aspect_ratio.y * 2 - radiuses.x - radiuses.w), vec2(-aspect_ratio.x, radiuses.w - aspect_ratio.y))) ? vec4(0, 0, 0, 1) : pixel_type; // Left

	// Linear borders
	pixel_type = bool(rectangle(tex_coord, vec2(aspect_ratio.x * 2 - radiuses.x - radiuses.y, border.x), vec2(radiuses.x - aspect_ratio.x, aspect_ratio.y - border.x))) ? vec4(1, 0, 0, 0) : pixel_type; // Top
	pixel_type = bool(rectangle(tex_coord, vec2(aspect_ratio.x * 2 - radiuses.z - radiuses.w, border.z), vec2(radiuses.w - aspect_ratio.x, -aspect_ratio.y))) ? vec4(1, 0, 0, 0) : pixel_type; // Bottom
	pixel_type = bool(rectangle(tex_coord, vec2(border.w, aspect_ratio.y * 2 - radiuses.x - radiuses.w), vec2(-aspect_ratio.x, radiuses.w - aspect_ratio.y))) ? vec4(1, 0, 0, 0) : pixel_type; // Left
	pixel_type = bool(rectangle(tex_coord, vec2(border.y, aspect_ratio.y * 2 - radiuses.y - radiuses.z), vec2(aspect_ratio.x - border.y, radiuses.z - aspect_ratio.y))) ? vec4(1, 0, 0, 0) : pixel_type; // Right
	// FragColor += rectangle(tex_coord, vec2(), vec2());
	
	FragColor = (pixel_type.r == 1) ? border_color_v : FragColor;
	FragColor = (pixel_type.a == 1) ? color_v : FragColor;
	// FragColor = (pixel_type.g == 1) ? vec4(1, 0, 1, 1) : FragColor;
	// FragColor = vec4(vec3(tex_coord.xy, 1), 1);
}