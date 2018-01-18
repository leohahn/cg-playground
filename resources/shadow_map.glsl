/* ====================================
 *
 *   Vertex Shader
 *
 * ==================================== */
#ifdef COMPILING_VERTEX

layout (location = 0) in vec3 att_position;

///////////////////////////////////////////////////////////////////////
// TODO: See if it is possible to remove this unused attributes ///////
layout (location = 1) in vec2 att_tex_coords;
layout (location = 2) in vec3 att_normal;
layout (location = 3) in vec3 att_tangent;
layout (location = 4) in vec3 att_bitangent;
///////////////////////////////////////////////////////////////////////

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

void
main()
{
	// Leaving the shader empty is the same as typing what's below
	gl_FragDepth = gl_FragCoord.z;
}

#endif
