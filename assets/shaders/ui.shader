{
	name: "UI Shader",
	{
		stage: VERTEX,
		source: "
			#version 140
			#extension GL_ARB_explicit_attrib_location : enable
			layout (location = 0) in vec3 pos_a;

			// Instanced
			layout (location = 1) in vec3 offset_a;
			layout (location = 2) in vec2 scale_a;
			layout (location = 3) in vec3 color_a;
			layout (location = 4) in vec3 border_color_a;
			layout (location = 5) in ivec4 border_a;
			layout (location = 6) in vec4 texture_src_a;

			layout (std140) uniform ShaderGlobals{
				mat4 projection_persp;
				mat4 projection_ortho;
				mat4 view;
				float time;
			};

			uniform mat4 tex_coordinates;


			out vec2 tex_coordinate_v;
			flat out vec2 scale_v;
			flat out vec3 color_v;
			flat out vec3 border_color_v;
			flat out ivec4 border_v;
			flat out vec4 texture_src_f;

			void main(){
				gl_Position = projection_ortho * vec4(pos_a * vec3(scale_a, 1) + offset_a, 1);

				int quadVert = gl_VertexID % 4;
				tex_coordinate_v = vec2(tex_coordinates[quadVert][0], tex_coordinates[quadVert][1]);
				scale_v = scale_a;
				color_v = color_a;
				border_color_v = border_color_a;
				border_v = border_a;
				texture_src_f = texture_src_a;
			}
		",
	},
	{
		stage: FRAGMENT,
		source: "
			#version 140

			out vec4 FragColor;

			in vec2 tex_coordinate_v;
			flat in vec2 scale_v;
			flat in vec3 color_v;
			flat in vec3 border_color_v;
			flat in ivec4 border_v;
			flat in vec4 texture_src_f;

			uniform sampler2D src_texture;

			float random(vec2 st){
				return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
			}

			void main(){
				vec2 tex_coord = tex_coordinate_v;
				vec2 texture_size = textureSize(src_texture, 0);
				vec2 px_coord = tex_coord * scale_v;

				tex_coord *= texture_src_f.zw / texture_size;
				tex_coord += texture_src_f.xy / texture_size;

				// FragColor = texture(src_texture, tex_coord) * vec4(color_v, 1);
				// if(FragColor.a < 0.01){
				// 	discard;
				// }


				vec2 bl = step(border_v.xw,px_coord);       // bottom-left
				vec2 tr = step(border_v.zy,scale_v-px_coord);   // top-right
				// vec2 bl = step(vec2(0.1),tex_coord);       // bottom-left
				// vec2 tr = step(vec2(0.1),1-tex_coord);   // top-right
				vec3 shape = vec3(bl.x * bl.y * tr.x * tr.y);


				// FragColor = vec4(vec3(random(px_coord)), 1);
				// FragColor *= vec4(color_v, 1);
				FragColor = vec4(shape * color_v + (1-shape) * border_color_v, 1);
				// FragColor = vec4(vec3(), 1);
			}
		",
	},
}
