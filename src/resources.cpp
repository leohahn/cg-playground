#include "resources.hpp"
#include "lt/src/lt_utils.hpp"
#include "lt_math.hpp"
#include "glad/glad.h"
#include <cstring>
#include "gl_resources.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

lt_global_variable lt::Logger logger("resources");

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

static_assert(LT_Count(UNIT_CUBE_VERTICES) == LT_Count(UNIT_CUBE_TEX_COORDS),
			  "need the same amount of vertices");

const Vec3f UNIT_PLANE_VERTICES[] =
{
    Vec3f(-1.0f, -1.0f,  0.0f), // 0
    Vec3f( 1.0f, -1.0f,  0.0f), // 1
    Vec3f( 1.0f,  1.0f,  0.0f), // 2
    Vec3f(-1.0f,  1.0f,  0.0f), // 3
};

const Vec2f UNIT_PLANE_TEX_COORDS[] =
{
    Vec2f(0.0f, 0.0f), // 0
    Vec2f(1.0f, 0.0f), // 1
    Vec2f(1.0f, 1.0f), // 2
    Vec2f(0.0f, 1.0f), // 3
};

static_assert(LT_Count(UNIT_PLANE_VERTICES) == LT_Count(UNIT_PLANE_TEX_COORDS),
			  "vertices and tex_coords should be the same size");

lt_internal const u32 UNIT_PLANE_INDICES[] =
{
    0, 1, 2, 2, 3, 0
};

lt_internal void
setup_mesh_buffers_pu(Mesh &m)
{
	// Temporary vertex buffer to be deallocated at the end of the function.
	std::vector<Vertex_PU> vertexes_buf(m.vertices.size());
	for (usize i = 0; i < m.vertices.size(); i++)
	{
		vertexes_buf[i].position = m.vertices[i];
		vertexes_buf[i].tex_coords = m.tex_coords[i];
	}	

    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);
		
    glBindVertexArray(m.vao);

    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexes_buf.size() * sizeof(Vertex_PU), &vertexes_buf[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.faces.size() * sizeof(Face), &m.faces[0], GL_STATIC_DRAW);

    // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PU), (void*)offsetof(Vertex_PU, position));
    glEnableVertexAttribArray(0);
    // vertex texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PU), (void*)offsetof(Vertex_PU, tex_coords));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

lt_internal void
setup_mesh_buffers_p(Mesh &m)
{
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);

    glBufferData(GL_ARRAY_BUFFER, m.vertices.size() * sizeof(Vec3f), &m.vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.faces.size() * sizeof(Face), &m.faces[0], GL_STATIC_DRAW);

    // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

lt_internal void
setup_mesh_buffers_puntb(Mesh &m)
{
	// Temporary vertex buffer to be deallocated at the end of the function.
	std::vector<Vertex_PUNTB> vertexes_buf(m.vertices.size());
	for (usize i = 0; i < m.vertices.size(); i++)
	{
		vertexes_buf[i].position = m.vertices[i];
		vertexes_buf[i].tex_coords = m.tex_coords[i];
		vertexes_buf[i].normal = m.normals[i];
		vertexes_buf[i].tangent = m.tangents[i];
		vertexes_buf[i].bitangent = m.bitangents[i];
	}	
		
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);
		
    glBindVertexArray(m.vao);

    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexes_buf.size() * sizeof(Vertex_PUNTB), &vertexes_buf[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.faces.size() * sizeof(Face), &m.faces[0], GL_STATIC_DRAW);

    // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PUNTB),
						  (void*)offsetof(Vertex_PUNTB, position));
    glEnableVertexAttribArray(0);
    // vertex texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PUNTB),
						  (void*)offsetof(Vertex_PUNTB, tex_coords));
    glEnableVertexAttribArray(1);
    // vertex normals
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PUNTB),
						  (void*)offsetof(Vertex_PUNTB, normal));
    glEnableVertexAttribArray(2);
    // vertex tangents
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PUNTB),
						  (void*)offsetof(Vertex_PUNTB, tangent));
    glEnableVertexAttribArray(3);
    // vertex bitangents
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PUNTB),
						  (void*)offsetof(Vertex_PUNTB, bitangent));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);
}

Mesh *
Resources::load_cubemap(u32 cubemap_texture)
{
	const i64 this_mesh_id = get_new_id();

	Mesh *mesh = &meshes[this_mesh_id];
	mesh->id = this_mesh_id;
	mesh->vertices = std::vector<Vec3f>(LT_Count(UNIT_CUBE_VERTICES));

	for (usize i = 0; i < mesh->vertices.size(); i++)
		mesh->vertices[i] = UNIT_CUBE_VERTICES[i];

	for (usize i = 0; i < LT_Count(UNIT_CUBE_INDICES); i+=3)
	{
		Face face = {};
		face.val[0] = UNIT_CUBE_INDICES[i+2];
		face.val[1] = UNIT_CUBE_INDICES[i+1];
		face.val[2] = UNIT_CUBE_INDICES[i];
		mesh->faces.push_back(face);
	}

	Submesh sm = {};
	sm.start_index = 0;
	sm.num_indices = mesh->number_of_indices();
	sm.textures.push_back(Texture(cubemap_texture, "texture_cubemap"));
	mesh->submeshes.push_back(sm);

	setup_mesh_buffers_p(*mesh);
	return mesh;
}

Mesh *
Resources::load_shadow_map_render_surface(u32 shadow_map_texture)
{
	const i64 this_mesh_id = get_new_id();
	const isize NUM_VERTICES = LT_Count(UNIT_PLANE_VERTICES);
	const isize NUM_INDICES = LT_Count(UNIT_PLANE_INDICES);

	Mesh *mesh = &meshes[this_mesh_id];
	mesh->id = this_mesh_id;
	mesh->vertices = std::vector<Vec3f>(NUM_VERTICES);
	mesh->tex_coords = std::vector<Vec2f>(NUM_VERTICES);

	// Add all only the positions
	for (usize i = 0; i < NUM_VERTICES; i++)
	{
		mesh->vertices[i] = UNIT_PLANE_VERTICES[i];
		mesh->tex_coords[i] = UNIT_PLANE_TEX_COORDS[i];
	}

	for (usize i = 0; i < NUM_INDICES; i+=3)
	{
		Face face = {};
		face.val[0] = UNIT_PLANE_INDICES[i];
		face.val[1] = UNIT_PLANE_INDICES[i+1];
		face.val[2] = UNIT_PLANE_INDICES[i+2];

		mesh->faces.push_back(face);
		mesh->faces_textures.push_back(std::vector<isize>{0, 1, 2});
	}

	Submesh sm = {};
	sm.start_index = 0;
	sm.num_indices = mesh->number_of_indices();
	sm.textures.push_back(Texture(shadow_map_texture, "texture_shadow_map"));
	mesh->submeshes.push_back(sm);

	setup_mesh_buffers_pu(*mesh);
    return mesh;
}

// EFFICIENCY: Reuse the meshes that are exactly the same and are not going
// to change (e.g. shadow map surface and this quad).
Mesh *
Resources::load_hdr_render_quad(u32 hdr_texture)
{
	const i64 this_mesh_id = get_new_id();
	const isize NUM_VERTICES = LT_Count(UNIT_PLANE_VERTICES);
	const isize NUM_INDICES = LT_Count(UNIT_PLANE_INDICES);

	Mesh *mesh = &meshes[this_mesh_id];
	mesh->id = this_mesh_id;
	mesh->vertices = std::vector<Vec3f>(NUM_VERTICES);
	mesh->tex_coords = std::vector<Vec2f>(NUM_VERTICES);

	// Add all only the positions
	for (usize i = 0; i < NUM_VERTICES; i++)
	{
		mesh->vertices[i] = UNIT_PLANE_VERTICES[i];
		mesh->tex_coords[i] = UNIT_PLANE_TEX_COORDS[i];
	}

	for (usize i = 0; i < NUM_INDICES; i+=3)
	{
		Face face = {};
		face.val[0] = UNIT_PLANE_INDICES[i];
		face.val[1] = UNIT_PLANE_INDICES[i+1];
		face.val[2] = UNIT_PLANE_INDICES[i+2];

		mesh->faces.push_back(face);
		mesh->faces_textures.push_back(std::vector<isize>{0, 1, 2});
	}

	Submesh sm = {};
	sm.start_index = 0;
	sm.num_indices = mesh->number_of_indices();
	sm.textures.push_back(Texture(hdr_texture, "texture_hdr"));
	mesh->submeshes.push_back(sm);

	setup_mesh_buffers_pu(*mesh);
    return mesh;
}

Mesh *
Resources::load_unit_cube(u32 diffuse_texture, u32 specular_texture, u32 normal_texture)
{
	const i64 this_mesh_id = get_new_id();

	Mesh *mesh = &meshes[this_mesh_id];
	mesh->id = this_mesh_id;
	mesh->vertices = std::vector<Vec3f>(LT_Count(UNIT_CUBE_VERTICES));
	mesh->tex_coords = std::vector<Vec2f>(LT_Count(UNIT_CUBE_VERTICES));
	mesh->normals = std::vector<Vec3f>(LT_Count(UNIT_CUBE_VERTICES));
	mesh->tangents = std::vector<Vec3f>(LT_Count(UNIT_CUBE_VERTICES));
	mesh->bitangents = std::vector<Vec3f>(LT_Count(UNIT_CUBE_VERTICES));

	// Add all only the positions
	for (usize i = 0; i < LT_Count(UNIT_CUBE_VERTICES); i++)
	{
		mesh->vertices[i] = UNIT_CUBE_VERTICES[i];
		mesh->tex_coords[i] = UNIT_CUBE_TEX_COORDS[i];
	}

	for (usize i = 0; i < LT_Count(UNIT_CUBE_INDICES); i+=3)
	{
		Face face = {};
		face.val[0] = UNIT_CUBE_INDICES[i];
		face.val[1] = UNIT_CUBE_INDICES[i+1];
		face.val[2] = UNIT_CUBE_INDICES[i+2];

		// positions
		const Vec3f pos1 = mesh->vertices[face.val[0]];
		const Vec3f pos2 = mesh->vertices[face.val[1]];
		const Vec3f pos3 = mesh->vertices[face.val[2]];

		// texture-coords
		const Vec2f uv1 = mesh->tex_coords[face.val[0]];
		const Vec2f uv2 = mesh->tex_coords[face.val[1]];
		const Vec2f uv3 = mesh->tex_coords[face.val[2]];

		const Vec3f edge1 = pos2 - pos1;
		const Vec3f edge2 = pos3 - pos1;

		// Create the normal from the edges of the face
		const Vec3f normal = lt::normalize(lt::cross(edge1, edge2));

		mesh->normals[face.val[0]] = normal;
		mesh->normals[face.val[1]] = normal;
		mesh->normals[face.val[2]] = normal;

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

		mesh->tangents[face.val[0]] = tangent;
		mesh->tangents[face.val[1]] = tangent;
		mesh->tangents[face.val[2]] = tangent;

		mesh->bitangents[face.val[0]] = bitangent;
		mesh->bitangents[face.val[1]] = bitangent;
		mesh->bitangents[face.val[2]] = bitangent;

		LT_Assert(lt::cross(tangent, bitangent) == normal);

		mesh->faces.push_back(face);
		mesh->faces_textures.push_back(std::vector<isize>{0, 1, 2});
	}

	Submesh sm = {};
	sm.start_index = 0;
	sm.num_indices = mesh->number_of_indices();
	sm.textures.push_back(Texture(diffuse_texture, "material.texture_diffuse1"));
    sm.textures.push_back(Texture(specular_texture, "material.texture_specular1"));
	if (normal_texture)
		sm.textures.push_back(Texture(normal_texture, "material.texture_normal1"));
	mesh->submeshes.push_back(sm);

	setup_mesh_buffers_puntb(*mesh);
    return mesh;
}

Mesh *
Resources::load_unit_plane(f32 tex_coords_scale, u32 diffuse_texture,
						   u32 specular_texture, u32 normal_texture)
{
	const i64 this_mesh_id = get_new_id();

	Mesh *mesh = &meshes[this_mesh_id];
	mesh->id = this_mesh_id;
	mesh->vertices = std::vector<Vec3f>(LT_Count(UNIT_PLANE_VERTICES));
	mesh->tex_coords = std::vector<Vec2f>(LT_Count(UNIT_PLANE_VERTICES));
	mesh->normals = std::vector<Vec3f>(LT_Count(UNIT_PLANE_VERTICES));
	mesh->tangents = std::vector<Vec3f>(LT_Count(UNIT_PLANE_VERTICES));
	mesh->bitangents = std::vector<Vec3f>(LT_Count(UNIT_PLANE_VERTICES));

	// Add all only the positions
	for (usize i = 0; i < LT_Count(UNIT_PLANE_VERTICES); i++)
	{
		mesh->vertices[i] = UNIT_PLANE_VERTICES[i];
		mesh->tex_coords[i] = UNIT_PLANE_TEX_COORDS[i] * tex_coords_scale;
	}

	for (usize i = 0; i < LT_Count(UNIT_PLANE_INDICES); i+=3)
	{
		Face face = {};
		face.val[0] = UNIT_PLANE_INDICES[i];
		face.val[1] = UNIT_PLANE_INDICES[i+1];
		face.val[2] = UNIT_PLANE_INDICES[i+2];

		// positions
		const Vec3f pos1 = mesh->vertices[face.val[0]];
		const Vec3f pos2 = mesh->vertices[face.val[1]];
		const Vec3f pos3 = mesh->vertices[face.val[2]];

		// texture-coords
		const Vec2f uv1 = mesh->tex_coords[face.val[0]];
		const Vec2f uv2 = mesh->tex_coords[face.val[1]];
		const Vec2f uv3 = mesh->tex_coords[face.val[2]];

		const Vec3f edge1 = pos2 - pos1;
		const Vec3f edge2 = pos3 - pos1;

		const Vec3f normal = lt::normalize(lt::cross(edge1, edge2));

		// Update the normals on the vertices
		mesh->normals[face.val[0]] = normal;
		mesh->normals[face.val[1]] = normal;
		mesh->normals[face.val[2]] = normal;

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

		LT_Assert(lt::cross(tangent, bitangent) == normal);

		mesh->tangents[face.val[0]] = tangent;
		mesh->tangents[face.val[1]] = tangent;
		mesh->tangents[face.val[2]] = tangent;

		mesh->bitangents[face.val[0]] = bitangent;
		mesh->bitangents[face.val[1]] = bitangent;
		mesh->bitangents[face.val[2]] = bitangent;

		mesh->faces.push_back(face);
		mesh->faces_textures.push_back(std::vector<isize>{0, 1, 2});
	}
	Submesh sm = {};
	sm.start_index = 0;
	sm.num_indices = mesh->number_of_indices();
	sm.textures.push_back(Texture(diffuse_texture, "material.texture_diffuse1"));
    sm.textures.push_back(Texture(specular_texture, "material.texture_specular1"));
	if (normal_texture)
		sm.textures.push_back(Texture(normal_texture, "material.texture_normal1"));
	mesh->submeshes.push_back(sm);

	setup_mesh_buffers_puntb(*mesh);
    return mesh;
}

Mesh *
Resources::load_mesh_from_model(const char *path, u32 diffuse_texture, u32 specular_texture,
								u32 normal_texture, Resources &resources)
{
	using std::string;
	Assimp::Importer importer;

    const string fullpath = string(RESOURCES_PATH) + string(path);

	const aiScene *scene = importer.ReadFile(
		fullpath,
		aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices
		| aiProcess_FlipUVs
	);

	if (scene && scene->mRootNode && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
	{
		// For simplicity, assert that the number of meshes is one
		LT_Assert(scene->mNumMeshes == 1);

		const i64 new_id = get_new_id();
		Mesh *mesh = &meshes[new_id];

		aiMesh *ai_mesh = scene->mMeshes[0];

		// TODO: take into account the material information
		// ignore know since the .obj being used is not really well formed
		// aiString name;
		// aiMaterial *ai_material = scene->mMaterials[0];
		// ai_material->Get(AI_MATKEY_NAME, name);

		LT_Assert(ai_mesh->mTextureCoords[0]);

		for (isize i = 0; i < ai_mesh->mNumVertices; i++)
		{
			{
				aiVector3D v = ai_mesh->mVertices[i];
				mesh->vertices.push_back(Vec3f(v.x, v.y, v.z));
			}
			if (ai_mesh->mTextureCoords[0])
			{
				// FIXME: currently ignoring 3d texture coordinates
				aiVector3D t = ai_mesh->mTextureCoords[0][i];
				mesh->tex_coords.push_back(Vec2f(t.x, t.y));
			}
			if (ai_mesh->mNormals)
			{
				aiVector3D n = ai_mesh->mNormals[i];
				mesh->normals.push_back(Vec3f(n.x, n.y, n.z));
			}
			if (ai_mesh->mTangents)
			{
				aiVector3D t = ai_mesh->mTangents[i];
				mesh->tangents.push_back(Vec3f(t.x, t.y, t.z));
			}
			if (ai_mesh->mBitangents)
			{
				aiVector3D b = ai_mesh->mBitangents[i];
				mesh->bitangents.push_back(Vec3f(b.x, b.y, b.z));
			}
		}
		for (isize i = 0; i < ai_mesh->mNumFaces; i++)
		{
			aiFace ai_face = ai_mesh->mFaces[i];
			LT_Assert(ai_face.mNumIndices == 3);

			Face face;
			face.val[0] = ai_face.mIndices[0];
			face.val[1] = ai_face.mIndices[1];
			face.val[2] = ai_face.mIndices[2];

			mesh->faces.push_back(face);
		}

		LT_Assert(mesh->vertices.size());
		LT_Assert(mesh->tex_coords.size());
		LT_Assert(mesh->normals.size());
		LT_Assert(mesh->tangents.size());
		LT_Assert(mesh->bitangents.size());

		Submesh sm = {};
		sm.start_index = 0;
		sm.num_indices = mesh->number_of_indices();
		sm.textures.push_back(Texture(diffuse_texture, "material.texture_diffuse1"));
		sm.textures.push_back(Texture(specular_texture, "material.texture_specular1"));
		if (normal_texture)
			sm.textures.push_back(Texture(normal_texture, "material.texture_normal1"));
		mesh->submeshes.push_back(sm);

		setup_mesh_buffers_puntb(*mesh);
		return mesh;
	}
	else
	{
		logger.error("Failed to load the model: ", path);
		logger.error(importer.GetErrorString());
		return nullptr;
	}
	
}
