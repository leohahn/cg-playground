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

out vec4 frag_color;

in vec2 tex_coords;

uniform sampler2D texture_scene;
uniform sampler2D texture_bloom;

uniform bool enable_tone_mapping = true;
uniform bool enable_bloom = false;
uniform bool enable_gamma_correction = true;
uniform bool display_bloom_filter = false;
uniform float exposure = 1.0;

void
main()
{
	const float gamma = 2.2;
	vec3 result;

	if (display_bloom_filter)
	{
		vec3 hdr_color = texture(texture_bloom, tex_coords).rgb;
		result = vec3(1.0) - exp(-hdr_color * exposure);
	}
	else
	{
		vec3 hdr_color = texture(texture_scene, tex_coords).rgb;

		if (enable_bloom)
		{
			vec3 bloom_color = texture(texture_bloom, tex_coords).rgb;
			hdr_color += bloom_color;
		}

		if (enable_tone_mapping)
			result = vec3(1.0) - exp(-hdr_color * exposure);
		else
			result = hdr_color;
	}

	if (enable_gamma_correction)
		result = pow(result, vec3(1.0/gamma));

	frag_color = vec4(result, 1.0);
}

#endif
