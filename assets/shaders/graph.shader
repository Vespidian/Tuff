{
	name: "Mesh",
	{
		stage: VERTEX,
		source: "
			#version 140
			#extension GL_ARB_uniform_buffer_object : enable
			#extension GL_ARB_explicit_attrib_location : enable

			layout (location = 0) in vec3 pos_a;
			layout (location = 1) in vec3 normal_a;
			layout (location = 2) in vec2 texture_a;

			layout (location = 3) in vec3 offset;
			layout (location = 4) in vec3 clr;
			layout (location = 5) in vec3 scale;

			layout (std140) uniform ShaderGlobals{
				mat4 projection_persp;
				mat4 projection_ortho;
				mat4 view_matrix;
				float time;
			};
			flat out float time_f;
			uniform mat4 model;

			out vec3 vert_v;
			out vec3 normal_v;
			out vec2 texture_v;
			out vec3 clr_v;
			flat out int id;


			// Used by light shader
			out vec3 frag_pos_v;

			void main(){
				clr_v = clr;

				vert_v = pos_a;
				normal_v = normal_a;
				texture_v = texture_a;

				gl_Position = projection_ortho * vec4((pos_a + vec3(0, 10, 0)), 1);

				frag_pos_v = vec3(vec4(pos_a, 1));
				gl_PointSize = gl_Position.z;

			}
		",
	},
	{
		stage: FRAGMENT,

		source: "
			#version 130

			out vec4 FragColor;

			in vec3 vert_v;
			in vec3 normal_v;
			in vec2 texture_v;
			in vec3 clr_v;
			in vec3 frag_pos_v;


			void main(){

				FragColor = vec4(1, 1, 1, 1);
// 				FragColor = vec4(clr_v, 1);

			}
		",
	},
}


