#include "mesh.hpp"
#include <cstring>
#include "lt/src/lt_utils.hpp"

#include "glad/glad.h"

lt_internal lt::Logger logger("mesh");

Mesh::~Mesh()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}

// lt_internal void
// setup_mesh_buffers_p(Mesh &m)
// {
//     glGenVertexArrays(1, &m.vao);
//     glGenBuffers(1, &m.vbo);
//     glGenBuffers(1, &m.ebo);

//     glBindVertexArray(m.vao);
//     glBindBuffer(GL_ARRAY_BUFFER, m.vbo);

//     glBufferData(GL_ARRAY_BUFFER, m.vertices.size() * sizeof(Vec3f), &m.vertices[0], GL_STATIC_DRAW);

//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.faces.size() * sizeof(Face), &m.faces[0], GL_STATIC_DRAW);

//     // vertex positions
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), (void*)0);
//     glEnableVertexAttribArray(0);

//     glBindVertexArray(0);
// }

// lt_internal void
// setup_mesh_buffers_puntb(Mesh &m)
// {
// 	// Temporary vertex buffer to be deallocated at the end of the function.
// 	std::vector<Vertex_PUNTB> vertexes_buf(m.vertices.size());
// 	for (usize i = 0; i < m.vertices.size(); i++)
// 	{
// 		vertexes_buf[i].position = m.vertices[i];
// 		vertexes_buf[i].tex_coords = m.tex_coords[i];
// 		vertexes_buf[i].normal = m.normals[i];
// 		vertexes_buf[i].tangent = m.tangents[i];
// 		vertexes_buf[i].bitangent = m.bitangents[i];
// 	}	
		
//     glGenVertexArrays(1, &m.vao);
//     glGenBuffers(1, &m.vbo);
//     glGenBuffers(1, &m.ebo);
		
//     glBindVertexArray(m.vao);

//     glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
//     glBufferData(GL_ARRAY_BUFFER, vertexes_buf.size() * sizeof(Vertex_PUNTB), &vertexes_buf[0], GL_STATIC_DRAW);

//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.faces.size() * sizeof(Face), &m.faces[0], GL_STATIC_DRAW);

//     // vertex positions
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex_PUNTB, position));
//     glEnableVertexAttribArray(0);
//     // vertex texture coords
//     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex_PUNTB, tex_coords));
//     glEnableVertexAttribArray(1);
//     // vertex normals
//     glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex_PUNTB, normal));
//     glEnableVertexAttribArray(2);
//     // vertex tangents
//     glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex_PUNTB, tangent));
//     glEnableVertexAttribArray(3);
//     // vertex bitangents
//     glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex_PUNTB, bitangent));
//     glEnableVertexAttribArray(4);

//     glBindVertexArray(0);
// }

// Mesh
// make_mesh_cubemap(u32 cubemap_texture)
// {
// 	Mesh mesh = {};
// 	mesh.vertices = std::vector<Vec3f>(LT_Count(UNIT_CUBE_VERTICES));
// 	mesh.textures.push_back(Texture(cubemap_texture, "texture_cubemap"));

// 	for (usize i = 0; i < mesh.vertices.size(); i++)
// 		mesh.vertices[i] = UNIT_CUBE_VERTICES[i];

// 	for (usize i = 0; i < LT_Count(UNIT_CUBE_INDICES); i+=3)
// 	{
// 		Face face = {};
// 		face.val[0] = UNIT_CUBE_INDICES[i+2];
// 		face.val[1] = UNIT_CUBE_INDICES[i+1];
// 		face.val[2] = UNIT_CUBE_INDICES[i];
// 		mesh.faces.push_back(face);
// 	}

// 	setup_mesh_buffers_p(mesh);
// 	return mesh;
// }

// Mesh
// make_mesh_unit_cube(u32 diffuse_texture, u32 specular_texture, u32 normal_texture)
// {
// 	Mesh mesh = {};
// 	mesh.vertices = std::vector<Vec3f>(LT_Count(UNIT_CUBE_VERTICES));
// 	mesh.tex_coords = std::vector<Vec2f>(LT_Count(UNIT_CUBE_VERTICES));
// 	mesh.normals = std::vector<Vec3f>(LT_Count(UNIT_CUBE_VERTICES));
// 	mesh.tangents = std::vector<Vec3f>(LT_Count(UNIT_CUBE_VERTICES));
// 	mesh.bitangents = std::vector<Vec3f>(LT_Count(UNIT_CUBE_VERTICES));

// 	// Add all textures to the mesh
// 	mesh.textures.push_back(Texture(diffuse_texture, "texture_diffuse"));
//     mesh.textures.push_back(Texture(specular_texture, "texture_specular"));
// 	if (normal_texture)
// 		mesh.textures.push_back(Texture(normal_texture, "texture_normal"));

// 	// Add all only the positions
// 	for (usize i = 0; i < LT_Count(UNIT_CUBE_VERTICES); i++)
// 	{
// 		mesh.vertices[i] = UNIT_CUBE_VERTICES[i];
// 		mesh.tex_coords[i] = UNIT_CUBE_TEX_COORDS[i];
// 	}

// 	for (usize i = 0; i < LT_Count(UNIT_CUBE_INDICES); i+=3)
// 	{
// 		Face face = {};
// 		face.val[0] = UNIT_CUBE_INDICES[i];
// 		face.val[1] = UNIT_CUBE_INDICES[i+1];
// 		face.val[2] = UNIT_CUBE_INDICES[i+2];

// 		// positions
// 		const Vec3f pos1 = mesh.vertices[face.val[0]];
// 		const Vec3f pos2 = mesh.vertices[face.val[1]];
// 		const Vec3f pos3 = mesh.vertices[face.val[2]];

// 		// texture-coords
// 		const Vec2f uv1 = mesh.tex_coords[face.val[0]];
// 		const Vec2f uv2 = mesh.tex_coords[face.val[1]];
// 		const Vec2f uv3 = mesh.tex_coords[face.val[2]];

// 		const Vec3f edge1 = pos2 - pos1;
// 		const Vec3f edge2 = pos3 - pos1;

// 		// Create the normal from the edges of the face
// 		const Vec3f normal = lt::normalize(lt::cross(edge1, edge2));

// 		mesh.normals[face.val[0]] = normal;
// 		mesh.normals[face.val[1]] = normal;
// 		mesh.normals[face.val[2]] = normal;

// 		const Vec2f delta_uv1 = uv2 - uv1;
// 		const Vec2f delta_uv2 = uv3 - uv1;
// 		const f32 f = 1.0f / (delta_uv1.x*delta_uv2.y - delta_uv2.x*delta_uv1.y);

// 		Vec3f tangent;
// 		tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
// 		tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
// 		tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);
// 		tangent = lt::normalize(tangent);

// 		Vec3f bitangent;
// 		bitangent.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
// 		bitangent.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
// 		bitangent.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);
// 		bitangent = lt::normalize(bitangent);  

// 		mesh.tangents[face.val[0]] = tangent;
// 		mesh.tangents[face.val[1]] = tangent;
// 		mesh.tangents[face.val[2]] = tangent;

// 		mesh.bitangents[face.val[0]] = bitangent;
// 		mesh.bitangents[face.val[1]] = bitangent;
// 		mesh.bitangents[face.val[2]] = bitangent;

// 		LT_Assert(lt::cross(tangent, bitangent) == normal);

// 		mesh.faces.push_back(face);
// 		mesh.faces_textures.push_back(std::vector<isize>{0, 1, 2});
// 	}

// 	setup_mesh_buffers_puntb(mesh);
//     return mesh;
// }

// Mesh
// make_mesh_unit_plane(f32 tex_coords_scale, u32 diffuse_texture,
// 					 u32 specular_texture, u32 normal_texture)
// {
// 	// logger.log("Creating unit plane");
// 	Mesh mesh = {};
// 	mesh.vertices = std::vector<Vec3f>(LT_Count(UNIT_PLANE_VERTICES));
// 	mesh.tex_coords = std::vector<Vec2f>(LT_Count(UNIT_PLANE_VERTICES));
// 	mesh.normals = std::vector<Vec3f>(LT_Count(UNIT_PLANE_VERTICES));
// 	mesh.tangents = std::vector<Vec3f>(LT_Count(UNIT_PLANE_VERTICES));
// 	mesh.bitangents = std::vector<Vec3f>(LT_Count(UNIT_PLANE_VERTICES));

// 	// Add all textures to the mesh
// 	mesh.textures.push_back(Texture(diffuse_texture, "texture_diffuse"));
//     mesh.textures.push_back(Texture(specular_texture, "texture_specular"));
// 	if (normal_texture)
// 		mesh.textures.push_back(Texture(normal_texture, "texture_normal"));

// 	// Add all only the positions
// 	for (usize i = 0; i < LT_Count(UNIT_PLANE_VERTICES); i++)
// 	{
// 		mesh.vertices[i] = UNIT_PLANE_VERTICES[i];
// 		mesh.tex_coords[i] = UNIT_PLANE_TEX_COORDS[i] * tex_coords_scale;
// 	}

// 	for (usize i = 0; i < LT_Count(UNIT_PLANE_INDICES); i+=3)
// 	{
// 		Face face = {};
// 		face.val[0] = UNIT_PLANE_INDICES[i];
// 		face.val[1] = UNIT_PLANE_INDICES[i+1];
// 		face.val[2] = UNIT_PLANE_INDICES[i+2];

// 		// positions
// 		const Vec3f pos1 = mesh.vertices[face.val[0]];
// 		const Vec3f pos2 = mesh.vertices[face.val[1]];
// 		const Vec3f pos3 = mesh.vertices[face.val[2]];

// 		// texture-coords
// 		const Vec2f uv1 = mesh.tex_coords[face.val[0]];
// 		const Vec2f uv2 = mesh.tex_coords[face.val[1]];
// 		const Vec2f uv3 = mesh.tex_coords[face.val[2]];

// 		const Vec3f edge1 = pos2 - pos1;
// 		const Vec3f edge2 = pos3 - pos1;

// 		const Vec3f normal = lt::normalize(lt::cross(edge1, edge2));

// 		// Update the normals on the vertices
// 		mesh.normals[face.val[0]] = normal;
// 		mesh.normals[face.val[1]] = normal;
// 		mesh.normals[face.val[2]] = normal;

// 		const Vec2f delta_uv1 = uv2 - uv1;
// 		const Vec2f delta_uv2 = uv3 - uv1;
// 		const f32 f = 1.0f / (delta_uv1.x*delta_uv2.y - delta_uv2.x*delta_uv1.y);

// 		Vec3f tangent;
// 		tangent.x = f * (delta_uv2.y * edge1.x - delta_uv1.y * edge2.x);
// 		tangent.y = f * (delta_uv2.y * edge1.y - delta_uv1.y * edge2.y);
// 		tangent.z = f * (delta_uv2.y * edge1.z - delta_uv1.y * edge2.z);
// 		tangent = lt::normalize(tangent);

// 		Vec3f bitangent;
// 		bitangent.x = f * (-delta_uv2.x * edge1.x + delta_uv1.x * edge2.x);
// 		bitangent.y = f * (-delta_uv2.x * edge1.y + delta_uv1.x * edge2.y);
// 		bitangent.z = f * (-delta_uv2.x * edge1.z + delta_uv1.x * edge2.z);
// 		bitangent = lt::normalize(bitangent);  

// 		LT_Assert(lt::cross(tangent, bitangent) == normal);

// 		mesh.tangents[face.val[0]] = tangent;
// 		mesh.tangents[face.val[1]] = tangent;
// 		mesh.tangents[face.val[2]] = tangent;

// 		mesh.bitangents[face.val[0]] = bitangent;
// 		mesh.bitangents[face.val[1]] = bitangent;
// 		mesh.bitangents[face.val[2]] = bitangent;

// 		mesh.faces.push_back(face);
// 		mesh.faces_textures.push_back(std::vector<isize>{0, 1, 2});
// 	}

// 	setup_mesh_buffers_puntb(mesh);
//     return mesh;
// }

