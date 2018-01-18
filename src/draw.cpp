#include "draw.hpp"

#include "mesh.hpp"
#include "shader.hpp"
#include "gl_context.hpp"
#include "lt_utils.hpp"
#include "entities.hpp"
#include "camera.hpp"

lt_internal lt::Logger logger("draw");

#define LIGHT_MASK (ComponentKind_LightEmmiter | ComponentKind_Transform | ComponentKind_Renderable)
#define RENDER_MASK (ComponentKind_Renderable | ComponentKind_Transform)
#define SHADOW_CASTER_MASK (ComponentKind_ShadowCaster | ComponentKind_Renderable | ComponentKind_Transform)

ShadowMap
create_shadow_map(i32 width, i32 height, Shader *shader)
{
	ShadowMap sm = {};
	sm.width = width;
	sm.height = height;
	sm.shader = shader;
	   
	// Create the texture
	glGenTextures(1, &sm.texture);
	glBindTexture(GL_TEXTURE_2D, sm.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

	// Attach texture to the framebuffer
	glGenFramebuffers(1, &sm.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, sm.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sm.texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		logger.error("framebuffer not complete");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return sm;
}

void
draw_shadow_map(Mesh *mesh, Shader &shadow_map_render_shader, GLContext &context)
{
	using std::string;
	context.use_shader(shadow_map_render_shader);
	shadow_map_render_shader.set1i("shadow_map_texture", 0);

	glActiveTexture(GL_TEXTURE0); // activate proper texture unit before binding
				// retrieve texture number (the N in diffuse_textureN)
	string name = mesh->textures[0].type;
	LT_Assert(name == "texture_shadow_map");

	shadow_map_render_shader.set1i(name.c_str(), 0);
	glBindTexture(GL_TEXTURE_2D, mesh->textures[0].id);

	context.bind_vao(mesh->vao);
	glDrawElements(GL_TRIANGLES, mesh->number_of_indices(), GL_UNSIGNED_INT, 0);
	context.unbind_vao();

	glActiveTexture(GL_TEXTURE0);
}

void
draw_entities_for_shadow_map(const Entities &e, const Mat4f &light_view, const Vec3f &light_pos,
							 ShadowMap &shadow_map, GLContext &context)
{
	for (isize handle = 0; handle < MAX_ENTITIES; handle++)
	{
		if ((e.mask[handle] & SHADOW_CASTER_MASK) == SHADOW_CASTER_MASK)
		{
			// Draw lights first
			Mesh *mesh = e.renderable[handle].mesh;

			context.use_shader(*shadow_map.shader);
			shadow_map.shader->set_matrix("model", e.transform[handle].mat);

			context.bind_vao(mesh->vao);
			glDrawElements(GL_TRIANGLES, mesh->number_of_indices(), GL_UNSIGNED_INT, 0);
			context.unbind_vao();
		}
	}
}

void
draw_entities(const Entities &e, const Camera &camera, GLContext &context, ShadowMap &shadow_map)
{
	for (isize handle = 0; handle < MAX_ENTITIES; handle++)
	{
		if ((e.mask[handle] & LIGHT_MASK) == LIGHT_MASK)
		{
			// Draw lights first
			Shader *shader = e.renderable[handle].shader;
			Mesh *mesh = e.renderable[handle].mesh;

			context.use_shader(*shader);

			shader->set_matrix("model", e.transform[handle].mat);
			shader->set_matrix("view", camera.view_matrix());

			context.bind_vao(mesh->vao);
			glDrawElements(GL_TRIANGLES, mesh->number_of_indices(), GL_UNSIGNED_INT, 0);
			context.unbind_vao();

			LightEmmiter le = e.light_emmiter[handle];

			context.use_shader(*le.shader);
            for (isize i = 0; i < 4; ++i)
            {
				std::string i_str = std::to_string(i);
				le.shader->set3f(("point_lights["+i_str+"].position").c_str(), le.position);
				le.shader->set3f(("point_lights["+i_str+"].ambient").c_str(), le.ambient);
				le.shader->set3f(("point_lights["+i_str+"].diffuse").c_str(), le.diffuse);
				le.shader->set3f(("point_lights["+i_str+"].specular").c_str(), le.specular);
				le.shader->set1f(("point_lights["+i_str+"].constant").c_str(), le.constant);
				le.shader->set1f(("point_lights["+i_str+"].linear").c_str(), le.linear);
				le.shader->set1f(("point_lights["+i_str+"].quadratic").c_str(), le.quadratic);
			}
		}
		else if ((e.mask[handle] & RENDER_MASK) == RENDER_MASK)
		{
			using std::string;

			Shader *shader = e.renderable[handle].shader;
			Mesh *mesh = e.renderable[handle].mesh;

			context.use_shader(*shader);

			shader->set_matrix("model", e.transform[handle].mat);
			shader->set_matrix("view", camera.view_matrix());
            shader->set3f("view_position", camera.frustum.position);
            shader->set1f("material.shininess", e.renderable[handle].shininess);

			// Config the shadow map texture
			{
				u32 tex_unit = shader->texture_unit("texture_shadow_map");
				glActiveTexture(GL_TEXTURE0 + tex_unit);
				shader->set1i("texture_shadow_map", tex_unit);
				glBindTexture(GL_TEXTURE_2D, shadow_map.texture);
			}

			bool use_normal_map = false;

			for (usize i = 0; i < mesh->textures.size(); i++)
			{
				string name = mesh->textures[i].type;
				u32 texture_unit = shader->texture_unit(name);
				glActiveTexture(GL_TEXTURE0 + texture_unit);

				if (name == "material.texture_normal1")
					use_normal_map = true;

				shader->set1i(name.c_str(), texture_unit);
				glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
			}

			shader->set1i("material.use_normal_map", use_normal_map);

			// draw mesh
			context.bind_vao(mesh->vao);
			glDrawElements(GL_TRIANGLES, mesh->number_of_indices(), GL_UNSIGNED_INT, 0);
			context.unbind_vao();

			// always good practice to set everything back to defaults once configured.
			glActiveTexture(GL_TEXTURE0);
		}
	}
}

void
draw_skybox(const Mesh *mesh, Shader &shader, const Mat4f &view, GLContext &context)
{
	glDepthFunc(GL_LEQUAL);

	context.use_shader(shader);
	shader.set_matrix("view", view);

    context.bind_vao(mesh->vao);
	glActiveTexture(GL_TEXTURE0 + shader.texture_unit("skybox"));
	glBindTexture(GL_TEXTURE_CUBE_MAP, mesh->textures[0].id);
    glDrawElements(GL_TRIANGLES, mesh->number_of_indices(), GL_UNSIGNED_INT, 0);
    context.unbind_vao();

	glDepthFunc(GL_LESS);
}
