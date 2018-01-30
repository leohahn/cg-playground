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
uniform bool enable_gamma_correction = true;
uniform float exposure = 1.0;

void
main()
{
	const float gamma = 2.2;
	vec3 hdr_color = texture(texture_hdr, tex_coords).rgb;
	vec3 map;

	if (enable_tone_mapping)
	{
		map = vec3(1.0) - exp(-hdr_color *exposure);
	}
	else
		map = hdr_color;

	if (enable_gamma_correction)
	{
		map = pow(map, vec3(1.0/gamma));
	}

	frag_color = vec4(map, 1.0);
}

#endif
