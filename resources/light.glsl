/* ====================================
 *
 *   Vertex Shader
 *
 * ==================================== */
#ifdef COMPILING_VERTEX

layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
}

#endif

/* ====================================
 *
 *   Fragment Shader
 *
 * ==================================== */
#ifdef COMPILING_FRAGMENT

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 bright_color;

uniform vec3 light_color = vec3(1);
uniform float bloom_threshold = 1.0;

void
main()
{
    frag_color = vec4(light_color, 1);

	float brightness = dot(frag_color.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > bloom_threshold)
		bright_color = vec4(frag_color.rgb, 1.0);
	else
		bright_color = vec4(0.0, 0.0, 0.0, 1.0);
}

#endif
