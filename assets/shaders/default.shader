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
			layout (location = 5) in float scale;

			// layout (location = 4) in vec3 tangent_a;

			layout (std140) uniform ShaderGlobals{
				mat4 projection_persp;
				mat4 projection_ortho;
				mat4 view_matrix;
				float time;
			};
			flat out float time_f;
			// uniform float time;
			// uniform mat4 perspective_projection;
			// uniform mat4 view_matrix;
			uniform mat4 model;

			out vec3 vert_v;
			out vec3 normal_v;
			out vec2 texture_v;
			out mat3 TBN_v;
			out vec3 clr_v;
			flat out int id;

			// Used by light shader
			out vec3 frag_pos_v;
			uniform float value;
			float PHI = 1.61803398874989484820459;  // Φ = Golden Ratio  

			float gold_noise(in vec2 xy, in float seed){
				return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
			}

			void main(){
				// time_f = time;
				clr_v = clr;
				id = gl_InstanceID;
				
				// vec3 T = normalize(vec3(model * vec4(tangent_a, 0)));
				// vec3 B = normalize(vec3(model * vec4(cross(normal_a, tangent_a), 0)));
				// vec3 N = normalize(vec3(model * vec4(normal_a, 0)));
				// mat3 TBN = mat3(T, B, N);
				// vert_v = (projection * view_matrix * model * vec4(pos_a, 1)).xyz;
				vert_v = pos_a;
				normal_v = normal_a;
				texture_v = texture_a;
				// texture_v = mod(texture_v, 1.0);

				// float value = time * gold_noise(pos_a.xz, 502);
				// gl_Position = projection * view_matrix * model * vec4(pos_a + vec3(sin(value / 300.0) / 2), 1);
				// gl_Position = projection_persp * view_matrix * model * vec4(pos_a, 1);
				// float zval = (sin(time + pos_a.x*2 + pos_a.y*3) + 1)*0.2 + pos_a.z;
				// float yval = (sin(time + pos_a.x*5) + 1)*0.1 + pos_a.y;
				// gl_Position = projection_persp * view_matrix * model * vec4(pos_a.x, yval, zval, 1);
				gl_Position = projection_persp * view_matrix * vec4((pos_a*vec3(scale) + offset), 1);
				// gl_Position = projection * view_matrix * model * vec4(normal_a, 1);
				// gl_Position = projection * view_matrix * model * vec4(texture_a, 1);
				frag_pos_v = vec3(vec4(pos_a, 1));
				gl_PointSize = gl_Position.z;

			}
		",
	},
	{
		stage: FRAGMENT,
		
		uniforms:[
			{
				uniform: light_pos,
				description: "Position of the phong light",
				type: VEC3,
				
			},
			{
				uniform: light_color,
				description: "Color of the phong light",
				type: VEC3,
				default: [0.5, 0.25, 0.1],
			},
			{
				uniform: checker_size,
				description: "Size of the checkerboard pattern",
				type: FLOAT,
				default: 20.0,
				range: [5, 50],
			},
			{
				uniform: tex,
				description: "Base texture of mesh (currently unused)",
			},
			{
				uniform: normal_map,
				description: "Normal map texture",
			},
		],
		
		source: "
			#version 130

			out vec4 FragColor;

			// flat in float time_f;

			in vec3 vert_v;
			in vec3 normal_v;
			in vec2 texture_v;
			in mat3 TBN_v;
			in vec3 clr_v;
			uniform float value;
			flat in int id;

			in vec3 frag_pos_v;

			uniform sampler2D tex;
			uniform sampler2D normal_map;
			uniform vec3 light_pos;
			uniform vec3 light_color;
			uniform vec3 view_position;
			uniform float checker_size;
			uniform float normal_map_intensity;

			void main(){
				vec2 tv = mod(texture_v*2, 1.0);
				vec3 new_normal = normalize(normal_v.rgb);
				// vec3 new_normal = normalize(normal_v + (texture(normal_map, tv)).rgb);
				// vec3 new_normal = normalize(normal_v + (texture(tex, tv)).rgb);
				// new_normal *= 2.0 - 1.0;
				// new_normal = normalize(TBN_v * new_normal);
				// vec3 new_normal = normalize(normal_v);
				// vec3 new_normal = normalize((texture(normal_map, texture_v.xy)).rgb);

				vec3 ambient = vec3(0.25);
				vec3 sun_dir = normalize(radians(vec3(20, 45, 0)));
				// ambient += (dot(sun_dir, normalize(new_normal)) + 1) / 4;
				ambient += (dot(sun_dir, normalize(normal_v)) + 1) / 4;
				// if(dot(sun_dir, normalize(normal_v)) <= 0.0){
				// 	ambient = vec3(max(dot(sun_dir, normalize(normal_v)), 0) + 0.25);
				// }
				// vec3 sun = vec3(max(dot(sun_dir, normalize(normal_v)), 0));
				// float specular_strength = 0.5;
				float specular_strength = 1;

				// Calculate diffuse
				vec3 light_dir = normalize(light_pos - frag_pos_v);
				float difference = max(dot(new_normal, light_dir), 0);
				// vec3 light_color = vec3(1);
				vec3 diffuse = vec3(difference * 0.6);

				// Calculate specular
				vec3 view_dir = normalize(view_position - frag_pos_v);
				vec3 reflect_dir = reflect(-light_dir, new_normal);
				float spec = pow(max(dot(view_dir, reflect_dir), 0), 32);
				// vec3 specular = specular_strength * spec * light_color;

				//tmp
				reflect_dir = reflect(-sun_dir, new_normal);
				spec = pow(max(dot(view_dir, reflect_dir), 0), 32);
				// specular += vec3(specular_strength * spec);
				// vec3 specular = specular_strength * spec * light_color;
				// vec3 specular = vec3(specular_strength * spec);
				// FragColor = texture(tex, tv);
				// FragColor = vec4(light_pos, 1);
				
				
				
				FragColor = vec4(clr_v, 1);
				// FragColor = vec4(1);
				// FragColor *= vec4(diffuse + specular + ambient, 1);
				FragColor *= vec4(diffuse + ambient, 1);

				// if(id == 3){
				// 	FragColor = vec4(1, 1, 1, 1);
				// }


				float grid_size = checker_size;
				//FragColor *= vec4(vec3(mod(floor(tv.x * grid_size) + floor(tv.y * grid_size), 2.0) == 0 ? 0.8 : 1), 1);
				// FragColor *= vec4(vec3(mod(floor((tv.x + tv.y) * 50), 2.0) == 0), 1);
				// FragColor = vec4(abs(vert_v.zzz), 1);
				// FragColor = vec4(abs(vert_v.zzz), 1) + texture(tex, tv.xy);
				// FragColor = vec4(vec3(0.75), 1);
				// FragColor *= vec4(diffuse, 1);
				// FragColor *= vec4(diffuse + ambient, 1);
				// FragColor *= vec4(ambient + vec3(sin(time_f)*0.5, cos(time_f)*0.5, sin(time_f)*0.5 + cos(time_f)*0.5), 1);
				// FragColor = vec4(diffuse + specular + ambient, 1);
				// FragColor *= vec4(ambient + specular, 1);
				// FragColor = vec4(light_pos, 1);
				// FragColor = vec4(light_color, 1);


			}
		",
	},
}