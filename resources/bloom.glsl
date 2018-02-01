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

uniform sampler2D texture_image;

uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void
main()
{
	vec2 tex_offset = 1.0 / textureSize(texture_image, 0);
	vec3 result = texture(texture_image, tex_coords).rgb * weight[0];

	if (horizontal)
	{
		for (int i = 1; i < 5; i++)
		{
			result += texture(texture_image, tex_coords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
			result += texture(texture_image, tex_coords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
		}
	}
	else
	{
		for (int i = 1; i < 5; i++)
		{
			result += texture(texture_image, tex_coords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			result += texture(texture_image, tex_coords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
		}
	}
	frag_color = vec4(result, 1.0);
}

#endif
