/* ====================================
 *
 *   Vertex Shader
 *
 * ==================================== */
#ifdef COMPILING_VERTEX

layout (location = 0) in vec3 att_position;

uniform mat4 model;
uniform mat4 light_space;

void
main()
{
    gl_Position = light_space * model * vec4(att_position, 1.0f);
}

#endif

/* ====================================
 *
 *   Fragment Shader
 *
 * ==================================== */
#ifdef COMPILING_FRAGMENT

#define BIAS 0.01

void
main()
{
	// Leaving the shader empty is the same as typing what's below
	gl_FragDepth = gl_FragCoord.z;
	// Add bias to front facing fragments to mostly fix peter panning isues with the shadow.
	gl_FragDepth += gl_FrontFacing ? BIAS : 0.0;
}

#endif
