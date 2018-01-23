#include "entities.hpp"
#include "lt_utils.hpp"
#include "resources.hpp"
#include "debug_gui.hpp"

lt_global_variable lt::Logger logger("entities");

EntityHandle
Entities::create(u32 components_mask)
{
	for (EntityHandle handle = 0; handle < MAX_ENTITIES; handle++)
		if (mask[handle] == ComponentKind_None)
		{
			mask[handle] = components_mask;
			return handle;
		}
	
	logger.error("Cannot create more entities!");
	return -1;
}

void
Entities::destroy(EntityHandle handle)
{
	LT_Assert(handle < MAX_ENTITIES && handle >= 0);
	mask[handle] = ComponentKind_None;
}

EntityHandle
create_textured_cube(Entities *entities, Resources *resources, Shader *shader,
					 const Mat4f &transform, f32 shininess, u32 diffuse_texture,
					 u32 specular_texture, u32 normal_texture)
{
	EntityHandle h = entities->create(ComponentKind_Renderable |
									  ComponentKind_Transform |
									  ComponentKind_ShadowCaster);
	LT_Assert(h >= 0);

	entities->renderable[h].mesh = resources->load_unit_cube(diffuse_texture, specular_texture, normal_texture);
	entities->renderable[h].shader = shader;
	entities->renderable[h].shininess = shininess;
	entities->transform[h].mat = transform;
	entities->name[h] = std::string("cube_") + std::to_string(h);

	DebugGuiState::instance().entity_names_and_ids.push_back(std::make_pair(entities->name[h], h));
	return h;
}

EntityHandle
create_entity_from_model(Entities &entities, Resources &resources, const char *path, Shader *shader,
						 const Mat4f &transform, f32 shininess, u32 texture_diffuse, u32 texture_specular,
						 u32 texture_normal)
{
	EntityHandle h = entities.create(ComponentKind_Renderable |
									 ComponentKind_Transform |
									 ComponentKind_ShadowCaster);
	LT_Assert(h >= 0);

	entities.renderable[h].mesh = resources.load_mesh_from_model(path, texture_diffuse, texture_specular,
																 texture_normal, resources);
	entities.renderable[h].shader = shader;
	entities.renderable[h].shininess = shininess;
	entities.transform[h].mat = transform;

	std::string path_str(path);
	i32 dot_pos = path_str.find_first_of(".");
	entities.name[h] = path_str.substr(0, dot_pos) + "_" + std::to_string(h);

	DebugGuiState::instance().entity_names_and_ids.push_back(std::make_pair(entities.name[h], h));
	return h;
}

EntityHandle
create_point_light(Entities *entities, Resources *resources, Shader *shader, const Mat4f &transform,
				   const LightEmmiter &light_emmiter, u32 diffuse_texture, u32 specular_texture)
{
	EntityHandle h = entities->create(ComponentKind_Renderable |
									  ComponentKind_Transform |
									  ComponentKind_LightEmmiter);

	LT_Assert(h >= 0);
	entities->renderable[h].mesh = resources->load_unit_cube(diffuse_texture, specular_texture);
	entities->renderable[h].shader = shader;
	entities->transform[h].mat = transform;
	entities->light_emmiter[h] = light_emmiter;
	entities->name[h] = std::string("point_light_") + std::to_string(h);

	DebugGuiState::instance().entity_names_and_ids.push_back(std::make_pair(entities->name[h], h));
	return h;
}

EntityHandle
create_plane(Entities *entities, Resources *resources, Shader *shader, const Mat4f &transform,
			 f32 shininess, f32 tex_coords_scale, u32 diffuse_texture, u32 specular_texture, u32 normal_texture)
{
	EntityHandle h = entities->create(ComponentKind_Renderable |
									  ComponentKind_Transform |
									  ComponentKind_ShadowCaster);
	LT_Assert(h >= 0);

	entities->renderable[h].mesh = resources->load_unit_plane(tex_coords_scale, diffuse_texture,
															  specular_texture, normal_texture);
	entities->renderable[h].shader = shader;
	entities->renderable[h].shininess = shininess;
	entities->transform[h].mat = transform;
	entities->name[h] = std::string("plane_") + std::to_string(h);

	DebugGuiState::instance().entity_names_and_ids.push_back(std::make_pair(entities->name[h], h));
	return h;
}

EntityHandle
create_skybox(Entities *entities, Resources *resources, Shader *shader, u32 skybox_texture)
{
	EntityHandle h = entities->create(ComponentKind_Renderable);
	LT_Assert(h >= 0);

	entities->renderable[h].mesh = resources->load_cubemap(skybox_texture);
	entities->renderable[h].shader = shader;
	entities->name[h] = std::string("skybox_") + std::to_string(h);

	DebugGuiState::instance().entity_names_and_ids.push_back(std::make_pair(entities->name[h], h));
	return h;
}
