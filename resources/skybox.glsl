/* ====================================
 *
 *   Vertex Shader
 *
 * ==================================== */
#ifdef COMPILING_VERTEX

layout (location = 0) in vec3 att_position;

out vec3 tex_coords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	tex_coords = att_position;
    gl_Position = projection * mat4(mat3(view)) * vec4(att_position, 1.0);
	gl_Position = gl_Position.xyww;
}

#endif

/* ====================================
 *
 *   Fragment Shader
 *
 * ==================================== */
#ifdef COMPILING_FRAGMENT

in vec3 tex_coords;
out vec4 frag_color;

uniform samplerCube skybox;

void main()
{
    frag_color = texture(skybox, tex_coords);
}

#endif
