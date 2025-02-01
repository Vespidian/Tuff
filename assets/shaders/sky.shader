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

			flat out float x_angle;

			// Used by light shader
			out vec3 frag_pos_v;
			uniform float value;
			float PHI = 1.61803398874989484820459;  // Φ = Golden Ratio

			float gold_noise(in vec2 xy, in float seed){
				return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
			}

			void main(){
				clr_v = clr;

				vert_v = pos_a;
				normal_v = normal_a;
				texture_v = texture_a;

				gl_Position = projection_ortho * vec4((pos_a*vec3(800) + vec3(0, 0, -499)), 1);

				frag_pos_v = vec3(vec4(pos_a, 1));
				gl_PointSize = gl_Position.z;

				x_angle = atan(view_matrix[1][1], view_matrix[2][2]);

			}
		",
	},
	{
		stage: FRAGMENT,

		source: "
			#version 130

			out vec4 FragColor;

			#define PI 3.1415926535897

			in vec3 vert_v;
			in vec3 normal_v;
			in vec2 texture_v;
			in vec3 clr_v;
			in vec3 frag_pos_v;

			flat in float x_angle;

			float PHI = 1.61803398874989484820459;  // Φ = Golden Ratio

			float rand(vec2 c){
				return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
			}

			// Pseudo random number generator.
			float hash( vec2 a )
			{

				return fract( sin( a.x * 3433.8 + a.y * 3843.98 ) * 45933.8 );

			}

			// Value noise courtesy of BigWingz
			// check his youtube channel he has
			// a video of this one.
			// Succint version by FabriceNeyret
// 			float noise( vec2 U )
// 			{
// 				vec2 id = floor( U );
// 					U = fract( U );
// 				U *= U * ( 3. - 2. * U );
//
// 				vec2 A = vec2( hash(id)            , hash(id + vec2(0,1)) ),
// 					B = vec2( hash(id + vec2(1,0)), hash(id + vec2(1,1)) ),
// 					C = mix( A, B, U.x);
//
// 				return mix( C.x, C.y, U.y );
// 			}

			float noise(vec2 p, float freq ){
				float unit = 1/freq;
				vec2 ij = floor(p/unit);
				vec2 xy = mod(p,unit)/unit;
				//xy = 3.*xy*xy-2.*xy*xy*xy;
				xy = .5*(1.-cos(PI*xy));
				float a = rand((ij+vec2(0.,0.)));
				float b = rand((ij+vec2(1.,0.)));
				float c = rand((ij+vec2(0.,1.)));
				float d = rand((ij+vec2(1.,1.)));
				float x1 = mix(a, b, xy.x);
				float x2 = mix(c, d, xy.x);
				return mix(x1, x2, xy.y);
			}

// 			float pNoise(vec2 p, int res){
// 				float persistance = .5;
// 				float n = 0.;
// 				float normK = 0.;
// 				float f = 4.;
// 				float amp = 1.;
// 				int iCount = 0;
// 				for (int i = 0; i<50; i++){
// 					n+=amp*noise(p, f);
// 					f*=2.;
// 					normK+=amp;
// 					amp*=persistance;
// 					if (iCount == res) break;
// 					iCount++;
// 				}
// 				float nf = n/normK;
// 				return nf*nf*nf*nf;
// 			}
			vec2 random(vec2 uv) {
				return vec2(fract(sin(dot(uv.xy,
					vec2(12.9898,78.233))) * 43758.5453123));
			}

			float worley(vec2 uv, float columns, float rows) {

				vec2 index_uv = floor(vec2(uv.x * columns, uv.y * rows));
				vec2 fract_uv = fract(vec2(uv.x * columns, uv.y * rows));

				float minimum_dist = 1.0;

				for (int y= -1; y <= 1; y++) {
					for (int x= -1; x <= 1; x++) {
						vec2 neighbor = vec2(float(x),float(y));
						vec2 point = random(index_uv + neighbor);

						vec2 diff = neighbor + point - fract_uv;
						float dist = length(diff);
						minimum_dist = min(minimum_dist, dist);
					}
				}

				return minimum_dist;
			}

			void main(){

// 				FragColor = vec4(vec3(noise(texture_v*10)), 1);
// 				FragColor = vec4(vec3(atan(1000 * texture_v.x - 768*x_angle)/PI + 0.5), 1);
// 				FragColor = vec4(vec3(atan(100 * rand(vec2(texture_v.x + x_angle*0.1, texture_v.y)) - 99)/PI + 0.5), 1);
// 				FragColor = vec4(vec3(atan(100 * noise(vec2(texture_v.x + x_angle*0.1, texture_v.y), 10) - 99)/PI + 0.5), 1);

// 				FragColor = vec4(vec3(1-worley(texture_v, 20, 20)), 1);
				FragColor = vec4(vec3(atan(1000 * (1-worley(texture_v+vec2(x_angle, 0), 20, 20)) - 2000*rand(texture_v))/PI + 0.5), 1);
			}
		",
	},
}


