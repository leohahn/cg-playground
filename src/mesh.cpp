#include "mesh.hpp"
#include <cstring>
#include "lt/src/lt_utils.hpp"

#include "glad/glad.h"

lt_internal lt::Logger logger("mesh");

lt_internal const u32 UNIT_CUBE_INDICES[] =
{
	0,  1,  2,  2,  3,  0,
	4,  5,  6,  6,  7,  4,
	8,  9, 10, 10, 11,  8,
	12, 13, 14, 14, 15, 12,
	16, 17, 18, 18, 19, 16,
	20, 21, 22, 22, 23, 20,
};

lt_internal const Vec3f UNIT_CUBE_VERTICES[24] =
{
	// FRONT
    Vec3f(-1, -1,  1), // 0
    Vec3f( 1, -1,  1), // 1
    Vec3f( 1,  1,  1), // 2
	Vec3f(-1,  1,  1), // 3
	// BACK
    Vec3f( 1, -1, -1), // 4
    Vec3f(-1, -1, -1), // 5
    Vec3f(-1,  1, -1), // 6
	Vec3f( 1,  1, -1), // 7
	// TOP
	Vec3f(-1,  1,  1), // 8
	Vec3f( 1,  1,  1), // 9
	Vec3f( 1,  1, -1), // 10
	Vec3f(-1,  1, -1), // 11
	// BOTTOM
	Vec3f( 1, -1,  1), // 12
	Vec3f(-1, -1,  1), // 13
	Vec3f(-1, -1, -1), // 14
	Vec3f( 1, -1, -1), // 15
	// LEFT
	Vec3f(-1, -1, -1), // 16
	Vec3f(-1, -1,  1), // 17
	Vec3f(-1,  1,  1), // 18
	Vec3f(-1,  1, -1), // 19
	// RIGHT
	Vec3f( 1, -1,  1), // 20
	Vec3f( 1, -1, -1), // 21
	Vec3f( 1,  1, -1), // 22
	Vec3f( 1,  1,  1), // 23
};

lt_internal const Vec2f UNIT_CUBE_TEX_COORDS[24] =
{
	// FRONT
    Vec2f(0, 0), // 0
    Vec2f(1, 0), // 1
    Vec2f(1, 1), // 2
    Vec2f(0, 1), // 3
	// BACK
    Vec2f(0, 0), // 4
    Vec2f(1, 0), // 5
    Vec2f(1, 1), // 6
    Vec2f(0, 1), // 7
	// TOP
    Vec2f(0, 0), // 8
    Vec2f(1, 0), // 9
    Vec2f(1, 1), // 10
    Vec2f(0, 1), // 11
	// BOTTOM
    Vec2f(0, 0), // 12
    Vec2f(1, 0), // 13
    Vec2f(1, 1), // 14
    Vec2f(0, 1), // 15
	// LEFT
    Vec2f(0, 0), // 16
    Vec2f(1, 0), // 17
    Vec2f(1, 1), // 18
    Vec2f(0, 1), // 19
	// RIGHT
    Vec2f(0, 0), // 20
    Vec2f(1, 0), // 21
    Vec2f(1, 1), // 22
    Vec2f(0, 1), // 23
};

const Vec3f UNIT_PLANE_VERTICES[] =
{
    Vec3f(-1.0f,  0.0f,  1.0f), // 0
    Vec3f( 1.0f,  0.0f,  1.0f), // 1
    Vec3f( 1.0f,  0.0f, -1.0f), // 2
    Vec3f(-1.0f,  0.0f, -1.0f), // 3
};

const Vec2f UNIT_PLANE_TEX_COORDS[] =
{
    Vec2f(0.0f, 0.0f), // 0
    Vec2f(1.0f, 0.0f), // 1
    Vec2f(1.0f, 1.0f), // 2
    Vec2f(0.0f, 1.0f), // 3
};

lt_internal const u32 UNIT_PLANE_INDICES[] =
{
    0, 1, 2, 2, 3, 0
};

lt_internal void
setup_mesh_buffers(Mesh &m)
{
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);

    glBufferData(GL_ARRAY_BUFFER, m.vertexes.size() * sizeof(Vertex), &m.vertexes[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.faces.size() * sizeof(Face), &m.faces[0], GL_STATIC_DRAW);

    // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    // vertex texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
    glEnableVertexAttribArray(1);
    // vertex normals
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    // vertex tangents
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    glEnableVertexAttribArray(3);
    // vertex bitangents
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);
}

lt_internal void
calc_tangent_bitangent(Vec3f edge1, Vec3f edge2, Vec2f delta_uv1, Vec2f delta_uv2, 
					   Vec3f &tangent, Vec3f &bitangent)
{
	const f32 f = 1.0f / (delta_uv1.x*delta_uv2.y - delta_uv2.x*delta_uv1.y);

	tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
	tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
	tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);
	tangent = lt::normalize(tangent);

	bitangent.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
	bitangent.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
	bitangent.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);
	bitangent = lt::normalize(bitangent);  
}

Mesh
Mesh::static_unit_cube(u32 diffuse_texture, u32 specular_texture, u32 normal_texture)
{
	Mesh mesh = {};

	// Add all textures to the mesh
	mesh.textures.push_back(Texture(diffuse_texture, "texture_diffuse"));
    mesh.textures.push_back(Texture(specular_texture, "texture_specular"));
	if (normal_texture)
		mesh.textures.push_back(Texture(normal_texture, "texture_normal"));

	// Add all only the positions
	LT_Assert(LT_Count(UNIT_CUBE_VERTICES) == LT_Count(UNIT_CUBE_TEX_COORDS));
	for (usize i = 0; i < LT_Count(UNIT_CUBE_VERTICES); i++)
	{
		Vertex vertex = {};
		vertex.position = UNIT_CUBE_VERTICES[i];
		vertex.tex_coords = UNIT_CUBE_TEX_COORDS[i];
		mesh.vertexes.push_back(vertex);
	}

	for (usize i = 0; i < LT_Count(UNIT_CUBE_INDICES); i+=3)
	{
		logger.log("Face ", i/3 + 1);
		Face face = {};
		face.val[0] = UNIT_CUBE_INDICES[i];
		face.val[1] = UNIT_CUBE_INDICES[i+1];
		face.val[2] = UNIT_CUBE_INDICES[i+2];

		// positions
		const Vec3f pos1 = mesh.vertexes[face.val[0]].position;
		const Vec3f pos2 = mesh.vertexes[face.val[1]].position;
		const Vec3f pos3 = mesh.vertexes[face.val[2]].position;

		// texture-coords
		const Vec2f uv1 = mesh.vertexes[face.val[0]].tex_coords;
		const Vec2f uv2 = mesh.vertexes[face.val[1]].tex_coords;
		const Vec2f uv3 = mesh.vertexes[face.val[2]].tex_coords;

		const Vec3f edge1 = pos2 - pos1;
		const Vec3f edge2 = pos3 - pos1;

		// Create the normal from the edges of the face
		Vec3f normal = lt::normalize(lt::cross(edge1, edge2));

		logger.log("    normal    = ", normal.x, " ", normal.y, " ", normal.z);
		mesh.vertexes[face.val[0]].normal = normal;
		mesh.vertexes[face.val[1]].normal = normal;
		mesh.vertexes[face.val[2]].normal = normal;

		const Vec2f delta_uv1 = uv2 - uv1;
		const Vec2f delta_uv2 = uv3 - uv1;
		const f32 f = 1.0f / (delta_uv1.x*delta_uv2.y - delta_uv2.x*delta_uv1.y);

		Vec3f tangent;
		tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
		tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
		tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);
		tangent = lt::normalize(tangent);

		Vec3f bitangent;
		bitangent.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
		bitangent.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
		bitangent.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);
		bitangent = lt::normalize(bitangent);  

		mesh.vertexes[face.val[0]].tangent = tangent;
		mesh.vertexes[face.val[1]].tangent = tangent;
		mesh.vertexes[face.val[2]].tangent = tangent;

		mesh.vertexes[face.val[0]].bitangent = bitangent;
		mesh.vertexes[face.val[1]].bitangent = bitangent;
		mesh.vertexes[face.val[2]].bitangent = bitangent;

		logger.log("    tangent   = ", tangent.x, " ", tangent.y, " ", tangent.z);
		logger.log("    bitangent = ", bitangent.x, " ", bitangent.y, " ", bitangent.z);

		// LT_Assert(lt::cross(tangent, bitangent) == normal);

		mesh.faces.push_back(face);
		mesh.faces_textures.push_back(std::vector<isize>{0, 1, 2});
	}

	setup_mesh_buffers(mesh);
    return mesh;
}

Mesh
Mesh::static_unit_plane(f32 tex_coords_scale, u32 diffuse_texture,
						u32 specular_texture, u32 normal_texture)
{
	logger.log("Creating unit plane");
	Mesh mesh = {};

	// Add all textures to the mesh
	mesh.textures.push_back(Texture(diffuse_texture, "texture_diffuse"));
    mesh.textures.push_back(Texture(specular_texture, "texture_specular"));
	if (normal_texture)
		mesh.textures.push_back(Texture(normal_texture, "texture_normal"));

	// Add all only the positions
	for (usize i = 0; i < LT_Count(UNIT_PLANE_VERTICES); i++)
	{
		Vertex vertex = {};
		vertex.position = UNIT_PLANE_VERTICES[i];
		vertex.tex_coords = UNIT_PLANE_TEX_COORDS[i] * tex_coords_scale;
		mesh.vertexes.push_back(vertex);
	}

	for (usize i = 0; i < LT_Count(UNIT_PLANE_INDICES); i+=3)
	{
		Face face = {};
		face.val[0] = UNIT_PLANE_INDICES[i];
		face.val[1] = UNIT_PLANE_INDICES[i+1];
		face.val[2] = UNIT_PLANE_INDICES[i+2];

		// positions
		const Vec3f pos1 = mesh.vertexes[face.val[0]].position;
		const Vec3f pos2 = mesh.vertexes[face.val[1]].position;
		const Vec3f pos3 = mesh.vertexes[face.val[2]].position;

		// texture-coords
		const Vec2f uv1 = mesh.vertexes[face.val[0]].tex_coords;
		const Vec2f uv2 = mesh.vertexes[face.val[1]].tex_coords;
		const Vec2f uv3 = mesh.vertexes[face.val[2]].tex_coords;

		const Vec3f edge1 = pos2 - pos1;
		const Vec3f edge2 = pos3 - pos1;

		const Vec3f normal = lt::normalize(lt::cross(edge1, edge2));

		// Update the normals on the vertices
		mesh.vertexes[face.val[0]].normal = normal;
		mesh.vertexes[face.val[1]].normal = normal;
		mesh.vertexes[face.val[2]].normal = normal;

		const Vec2f delta_uv1 = uv2 - uv1;
		const Vec2f delta_uv2 = uv3 - uv1;
		const f32 f = 1.0f / (delta_uv1.x*delta_uv2.y - delta_uv2.x*delta_uv1.y);

		Vec3f tangent;
		tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
		tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
		tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);
		tangent = lt::normalize(tangent);

		Vec3f bitangent;
		bitangent.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
		bitangent.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
		bitangent.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);
		bitangent = lt::normalize(bitangent);  

		mesh.vertexes[face.val[0]].tangent = tangent;
		mesh.vertexes[face.val[1]].tangent = tangent;
		mesh.vertexes[face.val[2]].tangent = tangent;

		mesh.vertexes[face.val[0]].bitangent = bitangent;
		mesh.vertexes[face.val[1]].bitangent = bitangent;
		mesh.vertexes[face.val[2]].bitangent = bitangent;

		mesh.faces.push_back(face);
		mesh.faces_textures.push_back(std::vector<isize>{0, 1, 2});

		logger.log("    normal    = ", normal.x, " ", normal.y, " ", normal.z);
		logger.log("    tangent   = ", tangent.x, " ", tangent.y, " ", tangent.z);
		logger.log("    bitangent = ", bitangent.x, " ", bitangent.y, " ", bitangent.z);
		LT_Assert(lt::cross(tangent, bitangent) == normal);
	}

	setup_mesh_buffers(mesh);
    return mesh;
}

