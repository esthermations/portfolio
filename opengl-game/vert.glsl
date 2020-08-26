#version 330 core

/*
  NOTE: We have u_vertex_xform and u_camera_view separate here, which is not
  standard procedure. Usually they're multiplied together on the CPU, since they
  only change per-VAO. It saves quite a few GPU matrix multiplications.
*/

uniform mat4 u_modelview;
uniform mat3 u_normal_xform;
uniform mat4  u_projection;
uniform bool  u_procedural_height;
uniform float u_time;

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoord;

out vec4 t_vertex;
out vec4 t_original_vertex;
out vec3 t_normal;
out vec2 t_texcoord;

float
rand(vec2 n)
{
	return 0.5
	       + 0.5 * fract(sin(n.x * 12.9898 + n.y * 78.233) * 43758.5453);
}

void
main(void)
{
	vec4 v = vec4(a_vertex, 1.0);
	vec3 n = a_normal;

	if (u_procedural_height) {
		if (v.x > 0.3 && v.x < 0.7) {
			v.y = 0;
		} else if (v.z > 0.99 || v.z < 0.01) {
			// Seams between terrain blocks
			v.y = 0;
		} else {
			v.y = 0.05 * rand(a_vertex.xz);
		}
	}

	t_vertex          = u_modelview * v;
	t_original_vertex = v;
	t_normal          = normalize(u_normal_xform * n);
	t_texcoord        = vec2(a_texcoord.x, -a_texcoord.y);

	gl_Position = u_projection * u_modelview * v;
}
