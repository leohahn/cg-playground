/* ====================================
 *
 *   Vertex Shader
 *
 * ==================================== */
#ifdef COMPILING_VERTEX

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in vec3 in_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 frag_tex_coord;
out vec3 frag_normal;
out vec3 frag_world_pos;

void
main()
{
    frag_tex_coord = in_tex_coord;
    frag_normal = mat3(transpose(inverse(model))) * in_normal;
    frag_world_pos = vec3(model * vec4(in_position, 1.0f));
    gl_Position = projection * view * model * vec4(in_position, 1.0f);
}

#endif

/* ====================================
 *
 *   Fragment Shader
 *
 * ==================================== */
#ifdef COMPILING_FRAGMENT

in vec2 frag_tex_coord;
in vec3 frag_normal;
in vec3 frag_world_pos;

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

vec3
calc_point_light(PointLight light, vec3 normal)
{
    vec3 diffuse_color = vec3(texture(material.texture_diffuse1, frag_tex_coord));
    vec3 specular_color = vec3(texture(material.texture_specular1, frag_tex_coord));

    vec3 frag_to_light = normalize(light.position - frag_world_pos);
    vec3 frag_to_view = normalize(view_position - frag_world_pos);
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
	if (material.use_normal_map)
	{
		normal = texture(material.texture_normal1, frag_tex_coord).rgb;
		normal = normalize(normal * 2 - 1.0); // map to range [-1, 1]
	}
	else
	{
		normal = normalize(frag_normal);
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
