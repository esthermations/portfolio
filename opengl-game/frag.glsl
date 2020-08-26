#version 330 core

// Redeclarations from vert shader...
uniform bool  u_procedural_height;
uniform float u_time;

float
rand(vec2 n)
{
	return 0.5
	       + 0.5 * fract(sin(n.x * 12.9898 + n.y * 78.233) * 43758.5453);
}

uniform vec4 u_light_position;
uniform vec3 u_light_ambient;
uniform vec3 u_light_diffuse;
uniform vec3 u_light_specular;

uniform vec3 u_mtl_ambient;
uniform vec3 u_mtl_diffuse;
uniform vec3  u_mtl_specular;
uniform float u_mtl_shininess;

uniform sampler2D u_diffuse_map;
uniform sampler2D u_specular_map;

in vec4 t_vertex;
in vec4 t_original_vertex;
in vec3 t_normal;
in vec2 t_texcoord;

out vec4 o_Frag_Colour;

vec4
phong_point_light(in vec4 pos, in vec3 norm)
{
	vec3 s = normalize(vec3(u_light_position - pos));
	vec3 r = reflect(-s, norm);

	// Ambient
	vec3 ambient = u_light_ambient * u_mtl_ambient;

	// Diffuse
	float s_dot_n = max(dot(s, norm), 0);
	vec3  diffuse = u_light_diffuse
		       * vec3(texture(u_diffuse_map, t_texcoord)) * s_dot_n;

	// Specular
	vec3 specular = vec3(0);
	if (s_dot_n > 0) {
		vec3 v   = normalize(-pos.xyz);
		specular = u_light_specular
			   * vec3(texture(u_specular_map, t_texcoord))
			   * pow(max(dot(r, v), 0.0), u_mtl_shininess);
	}

	return vec4(ambient + diffuse + specular, 1.0);
}

bool
between(float low, float middle, float high)
{
	return (low <= middle && middle <= high);
}

void
main(void)
{

	if (u_procedural_height) {
		vec4  colour; // colour
		vec4  v = t_original_vertex;
		float c = 0.05 + 0.2 * rand(v.xz + u_time);

		if (v.x > 0.35 && v.x < 0.65) {

			if ((0.49 < v.x && v.x < 0.51) && (0.10 < v.z && v.z < 0.40)) {
				// A yellow line in the middle of the road.
				o_Frag_Colour = vec4(2 * c, 2 * c, 0.5 * c, 1);
			} else {
				// The road, so grey.
				o_Frag_Colour = vec4(c, c, c, 1.0);
			}
		} else {
			// Grass, so greenish.
			o_Frag_Colour = vec4(0.1, 0.1 + c, 0.1, 1.0);
		}
	} else {
		// Non-procedural, just do light.
		o_Frag_Colour = phong_point_light(t_vertex, normalize(t_normal));
	}
}
