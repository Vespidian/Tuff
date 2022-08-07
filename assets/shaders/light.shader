{
	name: "Light Shader",
	{
		stage: VERTEX,
		source: "
			#version 140
			#extension GL_ARB_uniform_buffer_object : enable
			#extension GL_ARB_explicit_attrib_location : enable
			
			layout (location = 0) in vec3 pos_a;
			layout (location = 1) in vec3 normal_a;
			layout (location = 2) in vec2 texture_a;
			layout (location = 4) in vec3 tangent_a;

			layout (std140) uniform ShaderGlobals{
				mat4 projection_persp;
				mat4 projection_ortho;
				mat4 view;
				float time;
			};
			
			uniform mat4 model;
			
			void main(){
				gl_Position = projection_persp * view * model * vec4(pos_a, 1);
			}
		",
	},
	{
		stage: FRAGMENT,
		uniforms: [
			{
				uniform: light_color,
				type: VEC3,
				default: [0.1, 0.2, 0.8],
			},
		],
		source: "
			#version 130
			
			out vec4 FragColor;
			
			in vec3 vert_v;
			in vec3 normal_v;
			in vec2 texture_v;
			
			uniform vec3 light_color;
			
			void main(){
				// FragColor = vec4(1);
				FragColor = vec4(light_color, 1);
			}
		",
	},
}