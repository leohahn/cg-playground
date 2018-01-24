/* ====================================
 *
 *   Vertex Shader
 *
 * ==================================== */
#ifdef COMPILING_VERTEX

layout (location = 0) in vec3 att_position;
layout (location = 1) in vec2 att_tex_coords;
layout (location = 2) in vec3 att_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void
main()
{
	// vec3 new_pos = att_position + normalize(att_normal)*0.02;
	vec3 new_pos = att_position;
    gl_Position = projection * view * model * vec4(new_pos, 1.0f);
}

#endif

/* ====================================
 *
 *   Fragment Shader
 *
 * ==================================== */
#ifdef COMPILING_FRAGMENT

out vec4 frag_color;

void
main()
{
    frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}

#endif
