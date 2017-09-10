#include "draw.hpp"

#include "mesh.hpp"
#include "shader.hpp"
#include "gl_context.hpp"
#include "lt_utils.hpp"


lt_internal lt::Logger logger("draw");

void
draw_mesh(const Mesh& mesh, Shader& shader, GLContext& context)
{
    using std::string;

    u32 diffuse_nr = 1;
    u32 specular_nr = 1;

    for (usize i = 0; i < mesh.textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        string name = mesh.textures[i].type;
        string number;
        if (name == "texture_diffuse")
            number = std::to_string(diffuse_nr++);
        else if (name == "texture_specular")
            number = std::to_string(specular_nr++);
        else
            LT_Assert(false);

        shader.set1i(("material." + name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
    }

    // draw mesh
    context.bind_vao(mesh.vao);
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    context.unbind_vao();

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}
