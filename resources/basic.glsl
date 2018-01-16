/* ====================================
 *
 *   Vertex Shader
 *
 * ==================================== */
#ifdef COMPILING_VERTEX

layout (location = 0) in vec3 att_position;
layout (location = 1) in vec2 att_tex_coords;
layout (location = 2) in vec3 att_normal;
layout (location = 3) in vec3 att_tangent;
layout (location = 4) in vec3 att_bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT
{
	vec3 frag_world_pos;
	vec2 frag_tex_coords;
	mat3 TBN;
	vec3 frag_normal; // @Temporary
} vs_out;

void
main()
{
    vs_out.frag_tex_coords = att_tex_coords;
    vs_out.frag_world_pos = vec3(model * vec4(att_position, 1.0f));
    vs_out.frag_normal = mat3(transpose(inverse(model))) * att_normal;

	vec3 T = normalize(vec3(model * vec4(att_tangent,   0.0)));
	vec3 B = normalize(vec3(model * vec4(att_bitangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(att_normal,    0.0)));

	vs_out.TBN = mat3(T, B, N);

    gl_Position = projection * view * model * vec4(att_position, 1.0f);
}

#endif

/* ====================================
 *
 *   Fragment Shader
 *
 * ==================================== */
#ifdef COMPILING_FRAGMENT

// in vec2 frag_tex_coord;
// in vec3 frag_world_pos;

in VS_OUT
{
	vec3 frag_world_pos;
	vec2 frag_tex_coords;
	mat3 TBN;
	vec3 frag_normal;
} vs_out;

out vec4 frag_color;

const int NUM_POINT_LIGHTS = 1;

struct PointLight
{
    vec3  position;
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;

    float constant;
    float linear;
    float quadratic;
};

struct Material
{
    float      shininess;
    sampler2D  texture_diffuse1;
    sampler2D  texture_specular1;
	// whether to use texture_normal1 or not
	sampler2D  texture_normal1;
	bool       use_normal_map;
};

uniform vec3 view_position;
uniform PointLight point_lights[NUM_POINT_LIGHTS];
uniform Material material;
uniform mat4 model;

struct DebugGuiState
{
	bool enable_normal_mapping;
};

uniform DebugGuiState debug_gui_state;

vec3
calc_point_light(PointLight light, vec3 normal)
{
    vec3 diffuse_color = vec3(texture(material.texture_diffuse1, vs_out.frag_tex_coords));
    vec3 specular_color = vec3(texture(material.texture_specular1, vs_out.frag_tex_coords));

    vec3 frag_to_light = normalize(light.position - vs_out.frag_world_pos);
    vec3 frag_to_view = normalize(view_position - vs_out.frag_world_pos);
    vec3 halfway_dir = normalize(frag_to_light + frag_to_view);

    float dist = length(frag_to_light);
    float attenuation = 1.0 / (light.constant + light.linear*dist + light.quadratic*pow(dist, 2));

    vec3 ambient = light.ambient * diffuse_color * vec3(0.04f);

    float diffuse_strength = max(0.0f, dot(frag_to_light, normal));
    vec3 diffuse = light.diffuse * diffuse_strength * diffuse_color;

    float specular_strength = pow(max(0.0f, dot(halfway_dir, normal)), material.shininess);
    vec3 specular = light.specular * (specular_strength * specular_color);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void
main()
{
	vec3 normal;
	if (debug_gui_state.enable_normal_mapping && material.use_normal_map)
	{
		normal = texture(material.texture_normal1, vs_out.frag_tex_coords).rgb;
		normal = normalize(normal * 2 - 1.0); // map to range [-1, 1]
		normal = normalize(vs_out.TBN * normal);
	}
	else
	{
		normal = normalize(vs_out.frag_normal);
	}

    vec3 light_contributions = vec3(0);

    for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
        light_contributions += calc_point_light(point_lights[i], normal);

    frag_color = vec4(light_contributions, 1.0f);

    // Apply gamma correction
    float gamma = 2.2;
    frag_color.rgb = pow(frag_color.rgb, vec3(1.0/gamma));
}

#endif
