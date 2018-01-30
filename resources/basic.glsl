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
uniform mat4 light_space;

out VS_OUT
{
	vec3 frag_world_pos;
	vec2 frag_tex_coords;
	mat3 TBN;
	vec3 frag_normal; // @Temporary
	vec4 frag_pos_light_space;
} vs_out;

void
main()
{
    vs_out.frag_tex_coords = att_tex_coords;
    vs_out.frag_world_pos = vec3(model * vec4(att_position, 1.0f));
    vs_out.frag_normal = mat3(transpose(inverse(model))) * att_normal;
	vs_out.frag_pos_light_space = light_space * vec4(vs_out.frag_world_pos, 1.0f);

	vec3 T = normalize(vec3(model * vec4(att_tangent,   0.0)));
	vec3 B = normalize(vec3(model * vec4(att_bitangent, 0.0)));
	vec3 N = normalize(vs_out.frag_normal);

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

in VS_OUT
{
	vec3 frag_world_pos;
	vec2 frag_tex_coords;
	mat3 TBN;
	vec3 frag_normal;
	vec4 frag_pos_light_space;
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

struct DirectionalLight
{
	vec3  direction;
	vec3  ambient;
	vec3  diffuse;
	vec3  specular;
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
uniform DirectionalLight dir_light;
uniform Material material;
uniform mat4 model;

uniform sampler2D texture_shadow_map;

struct DebugGuiState
{
	bool enable_normal_mapping;
	float pcf_texel_offset;
	int pcf_window_side;
};

uniform DebugGuiState debug_gui_state;

float
shadow_calculation(vec4 pos_light_space, vec3 surface_normal, vec3 light_dir, DebugGuiState state)
{
	// TODO: Maybe expose this variables to the debug GUI
	const int mipmap_lvl = 0;
	float texel_offset = state.pcf_texel_offset;
	int window_side = state.pcf_window_side;
	int num_sampled_texels = window_side*window_side;
	int offset_xy = window_side/2;

	// perspective divide and map coordinates to the texture's one
	vec3 projection_coords = pos_light_space.xyz / pos_light_space.w;
	projection_coords = projection_coords * 0.5 + 0.5;

	// Return no shadow if the fragment is outside the far plane of the light space
	if (projection_coords.z > 1.0)
		return 0.0;

	// Implement Percentage-Closer Filtering
	float frag_depth = projection_coords.z;
	float shadow = 0.0;
	vec2 texel_size = texel_offset / textureSize(texture_shadow_map, mipmap_lvl);
	// Get depth values for a 3x3 neighborhood, then average by 9 (number of neighbors)
	for (int y = -offset_xy; y <= offset_xy; y++)
		for (int x = -offset_xy; x <= offset_xy; x++)
		{
			float depth = texture(texture_shadow_map, projection_coords.xy + vec2(x, y)*texel_size).r;
			shadow += float(frag_depth > depth);
		}
	shadow /= num_sampled_texels;
	return shadow;
}

vec3
calc_directional_light(DirectionalLight dir_light, vec3 normal, vec3 surface_normal, vec4 frag_pos_light_space)
{
    vec3 diffuse_color = vec3(texture(material.texture_diffuse1, vs_out.frag_tex_coords));
    vec3 specular_color = vec3(texture(material.texture_specular1, vs_out.frag_tex_coords));

    vec3 frag_to_light = -dir_light.direction;
    vec3 frag_to_view = normalize(view_position - vs_out.frag_world_pos);
    vec3 halfway_dir = normalize(frag_to_light + frag_to_view);

    vec3 ambient = dir_light.ambient * diffuse_color * vec3(0.04f);

	// NOTE: This is necessary because the normal from the normal map may have a positive dot product with the
	// frag_to_light vector even if the surface normal doesn't.
	float evaluate_normal_map = ceil(dot(surface_normal, frag_to_light));

    float diffuse_strength = max(0.0f, dot(frag_to_light, normal)) * evaluate_normal_map;
    vec3 diffuse = dir_light.diffuse * diffuse_strength * diffuse_color;

	float specular_strength = pow(max(0.0f, dot(halfway_dir, normal)), material.shininess) *
		evaluate_normal_map;
    vec3 specular = dir_light.specular * (specular_strength * specular_color);

	float shadow = shadow_calculation(frag_pos_light_space, surface_normal, dir_light.direction, debug_gui_state);
	// float shadow = 0;
    return (ambient + (diffuse + specular)*(1-shadow));
}

vec3
calc_point_light(PointLight light, vec3 normal, vec3 surface_normal)
{
    vec3 diffuse_color = vec3(texture(material.texture_diffuse1, vs_out.frag_tex_coords));
    vec3 specular_color = vec3(texture(material.texture_specular1, vs_out.frag_tex_coords));

    vec3 frag_to_light = normalize(light.position - vs_out.frag_world_pos);

    vec3 frag_to_view = normalize(view_position - vs_out.frag_world_pos);
    vec3 halfway_dir = normalize(frag_to_light + frag_to_view);

    float dist = length(frag_to_light);
    float attenuation = 1.0 / (light.constant + light.linear*dist + light.quadratic*pow(dist, 2));

    vec3 ambient = light.ambient * diffuse_color * vec3(0.04f);

	// NOTE: This is necessary because the normal from the normal map may have a positive dot product with the
	// frag_to_light vector even if the surface normal doesn't.
	float evaluate_normal_map = ceil(dot(surface_normal, frag_to_light));

    float diffuse_strength = max(0.0f, dot(frag_to_light, normal)) * evaluate_normal_map;
    vec3 diffuse = light.diffuse * diffuse_strength * diffuse_color;

	float specular_strength = pow(max(0.0f, dot(halfway_dir, normal)), material.shininess) *
		evaluate_normal_map;
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
	vec3 surface_normal = vs_out.frag_normal;

    for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
        light_contributions += calc_point_light(point_lights[i], normal, surface_normal);

	light_contributions += calc_directional_light(dir_light, normal, surface_normal, vs_out.frag_pos_light_space);

    frag_color = vec4(light_contributions, 1.0f);
}

#endif
