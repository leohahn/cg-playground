#include "draw.hpp"

#include "mesh.hpp"
#include "shader.hpp"
#include "gl_context.hpp"
#include "lt_utils.hpp"
#include "camera.hpp"
#include "application.hpp"
#include "debug_gui.hpp"

lt_internal lt::Logger logger("draw");

#define LIGHT_MASK (ComponentKind_LightEmmiter | ComponentKind_Transform | ComponentKind_Renderable)
#define RENDER_MASK (ComponentKind_Renderable | ComponentKind_Transform)
#define SHADOW_CASTER_MASK (ComponentKind_ShadowCaster | ComponentKind_Renderable | ComponentKind_Transform)

ShadowMap
create_shadow_map(i32 width, i32 height, Shader &shader)
{
	ShadowMap sm = {};
	sm.width = width;
	sm.height = height;
	sm.shader = &shader;
	   
	// Create the texture
	glGenTextures(1, &sm.texture);
	glBindTexture(GL_TEXTURE_2D, sm.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	const Vec4f border_color(1.0f, 1.0f, 1.0f, 1.0f);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);  
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &border_color.val[0]);  

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
draw_unit_quad(Mesh *mesh, Shader &shader, GLContext &context)
{
	using std::string;
	context.use_shader(shader);

	glActiveTexture(GL_TEXTURE0); // activate proper texture unit before binding
	context.bind_vao(mesh->vao);
	for (usize i = 0; i < mesh->submeshes.size(); i++)
	{
		Submesh sm = mesh->submeshes[i];
		LT_Assert(sm.textures.size() == 1);

		glBindTexture(GL_TEXTURE_2D, sm.textures[0].id);
		glDrawElements(GL_TRIANGLES, sm.num_indices, GL_UNSIGNED_INT, (const void*)sm.start_index);
	}
	context.unbind_vao();
}

void
draw_unit_quad_and_apply_bloom(const Application &app, Shader &render_shader,
							   Shader &bloom_shader, GLContext &context)
{
	bool horizontal = true;
	if (dgui::State::instance().enable_bloom)
	{
		bool first_iteration = true;
		i32 num_iterations = 10;

		context.use_shader(bloom_shader);
		for (i32 i = 0; i < num_iterations; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, app.pingpong_fbos[horizontal]);
			bloom_shader.set1i("horizontal", horizontal);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,
						  first_iteration ? app.bloom_texture : app.pingpong_textures[!horizontal]);

			context.bind_vao(app.render_quad->vao);
			glDrawElements(GL_TRIANGLES, app.render_quad->number_of_indices(), GL_UNSIGNED_INT, 0);
			context.unbind_vao();

			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	context.use_shader(render_shader);
	render_shader.set1i("display_bloom_filter", dgui::State::instance().display_bloom_filter);
	render_shader.set1i("enable_bloom", dgui::State::instance().enable_bloom);

	glActiveTexture(GL_TEXTURE0 + render_shader.texture_unit("texture_scene"));
	glBindTexture(GL_TEXTURE_2D, app.hdr_texture);

	if (dgui::State::instance().enable_bloom)
	{
		const i32 last_index = !horizontal;
		glActiveTexture(GL_TEXTURE0 + render_shader.texture_unit("texture_bloom"));
		glBindTexture(GL_TEXTURE_2D, app.pingpong_textures[last_index]);
	}

	context.bind_vao(app.render_quad->vao);
	glDrawElements(GL_TRIANGLES, app.render_quad->number_of_indices(), GL_UNSIGNED_INT, 0);
	context.unbind_vao();
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
draw_entities(f64 lag_offset, const Entities &e, const Camera &camera, GLContext &context,
			  ShadowMap &shadow_map, EntityHandle selected_entity)
{
	const Mat4f view_matrix = camera.view_matrix();

	for (EntityHandle handle = 0; handle < MAX_ENTITIES; handle++)
	{
		if ((e.mask[handle] & LIGHT_MASK) == LIGHT_MASK)
		{
			// Draw lights first
			Shader *shader = e.renderable[handle].shader;
			const Mesh *mesh = e.renderable[handle].mesh;
			const LightEmmiter le = e.light_emmiter[handle];

			context.use_shader(*shader);

			shader->set_matrix("model", e.transform[handle].mat);
			shader->set_matrix("view", view_matrix);
			shader->set3f("light_color", le.diffuse);
			shader->set1f("bloom_threshold", dgui::State::instance().bloom_threshold);

			if (handle == selected_entity)
				glStencilMask(0xff);

			context.bind_vao(mesh->vao);
			for (usize i = 0; i < mesh->submeshes.size(); i++)
			{
				Submesh sm = mesh->submeshes[i];
				glDrawElements(GL_TRIANGLES, sm.num_indices, GL_UNSIGNED_INT, (const void*)sm.start_index);
			}
			context.unbind_vao();

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
			shader->set_matrix("view", view_matrix);
            shader->set3f("view_position", camera.frustum.position);
            shader->set1f("material.shininess", e.renderable[handle].shininess);
			shader->set1f("bloom_threshold", dgui::State::instance().bloom_threshold);

			// Config the shadow map texture
			{
				u32 tex_unit = shader->texture_unit("texture_shadow_map");
				glActiveTexture(GL_TEXTURE0 + tex_unit);
				// shader->set1i("texture_shadow_map", tex_unit);
				glBindTexture(GL_TEXTURE_2D, shadow_map.texture);
			}

			if (handle == selected_entity)
				glStencilMask(0xff);

			context.bind_vao(mesh->vao);
			for (usize i = 0; i < mesh->submeshes.size(); i++)
			{
				bool use_normal_map = false;
				Submesh sm = mesh->submeshes[i];

				for (usize t = 0; t < sm.textures.size(); t++)
				{
					string name = sm.textures[t].type;
					u32 texture_unit = shader->texture_unit(name);
					glActiveTexture(GL_TEXTURE0 + texture_unit);

					if (name == "material.texture_normal1")
						use_normal_map = true;

					// shader->set1i(name.c_str(), texture_unit);
					glBindTexture(GL_TEXTURE_2D, sm.textures[t].id);
				}
				shader->set1i("material.use_normal_map", use_normal_map);

				glDrawElements(GL_TRIANGLES, sm.num_indices, GL_UNSIGNED_INT, (const void*)sm.start_index);
			}
			context.unbind_vao();
			glActiveTexture(GL_TEXTURE0);
		}
		glStencilMask(0x00);
	}
}

void
draw_skybox(const Mesh *mesh, Shader &shader, const Mat4f &view, GLContext &context)
{
	glDepthFunc(GL_LEQUAL);

	context.use_shader(shader);
	shader.set_matrix("view", view);

    context.bind_vao(mesh->vao);
	for (usize i = 0; i < mesh->submeshes.size(); i++)
	{
		Submesh sm = mesh->submeshes[i];
		LT_Assert(sm.textures.size() == 1);

		glActiveTexture(GL_TEXTURE0 + shader.texture_unit("skybox"));
		glBindTexture(GL_TEXTURE_CUBE_MAP, sm.textures[0].id);
		glDrawElements(GL_TRIANGLES, sm.num_indices, GL_UNSIGNED_INT, (const void*)sm.start_index);
	}
    context.unbind_vao();

	glDepthFunc(GL_LESS);
}

void
draw_selected_entity(const Entities &e, EntityHandle handle, Shader &selection_shader,
					 const Mat4f &view, GLContext &context)
{
	Mesh *mesh = e.renderable[handle].mesh;
	Mat4f transform = e.transform[handle].mat;

	Mat4f new_transform = lt::scale(transform, Vec3f(1.04f));

	// Draw selection upscaled with a simple shader
	context.use_shader(selection_shader);
	selection_shader.set_matrix("view", view);
	selection_shader.set_matrix("model", new_transform);

    context.bind_vao(mesh->vao);
	glDrawElements(GL_TRIANGLES, mesh->number_of_indices(), GL_UNSIGNED_INT, 0);
    context.unbind_vao();
}
