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
uniform bool enable_tone_mapping = true;

void
main()
{
	const float gamma = 2.2;
	vec3 hdr_color = texture(texture_hdr, tex_coords).rgb;

	if (enable_tone_mapping)
	{
		vec3 map = hdr_color / (hdr_color + vec3(1.0));
		map = pow(map, vec3(1.0/gamma));
		frag_color = vec4(map, 1.0);
	}
	else
	{
		vec3 map = hdr_color;
		map = pow(map, vec3(1.0/gamma));
		frag_color = vec4(map, 1.0);
	}
}

#endif
