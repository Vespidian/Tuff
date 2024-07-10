{
	name: "Axis",
	{
		stage: VERTEX,
		source: "
			/* --- AXIS VERTEX SHADER --- */
			#version 140
			#extension GL_ARB_uniform_buffer_object : enable
			#extension GL_ARB_explicit_attrib_location : enable

			layout (location = 0) in vec3 pos_a;
			layout (location = 1) in vec3 color_a;

			layout (std140) uniform ShaderGlobals{
				mat4 projection_persp;
				mat4 projection_ortho;
				mat4 view_matrix;
				float time;
			};

			out vec3 color_v;

			out vec3 pos_v;
			flat out vec3 pos_f;


			uniform mat4 model;

			void main(){
				color_v = color_a;
				gl_Position = projection_persp * view_matrix * model * vec4(pos_a, 1);


				pos_v = (projection_persp * view_matrix * model * vec4(pos_a, 1)).xyz;
				pos_f = (projection_persp * view_matrix * model * vec4(pos_a, 1)).xyz;

			}
		",
	}
	{
		stage: FRAGMENT,
		source: "
			/* --- AXIS FRAGMENT SHADER --- */
			#version 130

			out vec4 FragColor;

			in vec3 pos_v;
			flat in vec3 pos_f;

			uniform float zoom;

			in vec3 color_v;

			void main(){

				if(fract(length(pos_f - pos_v) * 400 / 20 / zoom) > 0.5){
					discard;
				}
				FragColor = vec4(color_v, 1);
			}
		",
	}
}