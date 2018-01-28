/* ====================================
 *
 *   Vertex Shader
 *
 * ==================================== */
#ifdef COMPILING_VERTEX

layout (location = 0) in vec3 att_position;
layout (location = 1) in vec2 att_tex_coords;

out vec2 tex_coords;

void
main()
{
    tex_coords = att_tex_coords;
    gl_Position = vec4(att_position, 1.0f);
}

#endif

/* ====================================
 *
 *   Fragment Shader
 *
 * ==================================== */
#ifdef COMPILING_FRAGMENT

in vec2 tex_coords;
out vec4 frag_color;

uniform sampler2D texture_hdr;

void
main()
{
	// frag_color = vec4(texture(texture_hdr, tex_coords).rgb, 1.0f);
	frag_color = texture(texture_hdr, tex_coords);
	// frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}

#endif
