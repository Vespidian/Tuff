{
	name: "default",
	{
		stage: VERTEX,
		source: "
			
			#version 140
			#extension GL_ARB_uniform_buffer_object : enable
			#extension GL_ARB_explicit_attrib_location : enable

			layout (location = 0) in vec3 pos_a;
			layout (location = 1) in vec3 color_a;

			layout (std140) uniform ShaderGlobals{
				mat4 projection_persp;
				mat4 projection_ortho;
				mat4 view;
				float time;
			};

			uniform vec3 grid_pos_u;
			uniform float value;

			out vec3 color_v;

			void main(){
				gl_Position = projection_persp * view * vec4(pos_a, 1);

				if(pos_a.z == 0){
					color_v = vec3(1, 0.2, 0.322);
				}else if(pos_a.x == 0){
					color_v = vec3(0.157, 0.565, 1);
				}else{
					color_v = color_a;
				}
			}
			
		",
	},
	{
		stage: FRAGMENT,
		uniforms:[
			{
				uniform: exposed_uni,
				description: "Test uniform",
			},
			{
				uniform: tint_u,
				description: "Tint", 
				type: VEC3, 
			},
			{
				uniform: seed_u,
				description: "Random Seed",
				default: 100,
				range: [0, 512],
			},
			{
				uniform: color_u,
				description: "Vector3 testing",
				default: [128, 56, 75];
				range: [0, 255];
			}
		],
		source: "
			
			#version 130
			out vec4 FragColor;
			
			uniform int seed_u;
			uniform vec3 color_u;
			uniform float value;
			uniform float exposed_uni;
			in vec3 color_v;

			void main(){
				
				FragColor = vec4(color_v, 1);
			}
			
		",
	},
}