#include "draw.hpp"
#include "tiles.hpp"
#include "shader.hpp"

void
draw_tile_map(const TileMap &tile_map)
{
    glBindVertexArray(tile_map.vao);


    shader_set(ShaderKind_Basic);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
