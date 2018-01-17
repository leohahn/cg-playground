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

void
draw_entities(const Entities &e, const Camera &camera, GLContext &context)
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

			shader->set_matrix("model", e.transform[handle].mat);
			shader->set_matrix("view", camera.view_matrix());
            shader->set3f("view_position", camera.frustum.position);
            shader->set1f("material.shininess", e.renderable[handle].shininess);

			context.use_shader(*shader);

			u32 diffuse_nr = 1;
			u32 specular_nr = 1;
			u32 normal_nr = 1;
			bool use_normal_map = false;

			for (usize i = 0; i < mesh->textures.size(); i++)
			{
				glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
				// retrieve texture number (the N in diffuse_textureN)
				string name = mesh->textures[i].type;
				string number;
				if (name == "texture_diffuse")
					number = std::to_string(diffuse_nr++);
				else if (name == "texture_specular")
					number = std::to_string(specular_nr++);
				else if (name == "texture_normal")
				{
					number = std::to_string(normal_nr++);
					use_normal_map = true;
				}
				else
					LT_Assert(false);

				shader->set1i(("material." + name + number).c_str(), i);
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
draw_mesh(const Mesh *mesh, Shader &shader, GLContext &context)
{
    using std::string;

    u32 diffuse_nr = 1;
    u32 specular_nr = 1;
    u32 normal_nr = 1;

	bool use_normal_map = false;

    for (usize i = 0; i < mesh->textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        string name = mesh->textures[i].type;
        string number;
        if (name == "texture_diffuse")
            number = std::to_string(diffuse_nr++);
        else if (name == "texture_specular")
            number = std::to_string(specular_nr++);
        else if (name == "texture_normal")
		{
			number = std::to_string(normal_nr++);
			use_normal_map = true;
		}
		else
            LT_Assert(false);

        shader.set1i(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
    }

	shader.set1i("material.use_normal_map", use_normal_map);

    // draw mesh
    context.bind_vao(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->number_of_indices(), GL_UNSIGNED_INT, 0);
    context.unbind_vao();

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}

void
draw_skybox(const Mesh *mesh, Shader &shader, const Mat4f &view, GLContext &context)
{
	glDepthFunc(GL_LEQUAL);

	context.use_shader(shader);
	shader.set_matrix("view", view);

    context.bind_vao(mesh->vao);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mesh->textures[0].id);
    glDrawElements(GL_TRIANGLES, mesh->number_of_indices(), GL_UNSIGNED_INT, 0);
    context.unbind_vao();

	glDepthFunc(GL_LESS);
}
