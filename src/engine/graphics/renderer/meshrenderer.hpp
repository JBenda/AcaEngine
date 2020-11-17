#include "meshrenderer.hpp"
#include "../core/opengl.hpp"
#include "../core/texture.hpp"
#include "mesh.hpp"


namespace graphics {

	MeshRenderer::MeshRenderer() : dirty(true) {
		//ggf. Programm

		glCall(glGenVertexArrays, 1, &vao);
		glCall(glBindVertexArray, vao);

		glCall(glGenBuffers, 1, &vbo);
		glCall(glBindBuffer, GL_ARRAY_BUFFER, vbo);

		glCall(glEnableVertexAttribArray, 0);
		glCall(glVertexAttribPointer, 0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)0);

		glCall(glEnableVertexAttribArray, 1);
		glCall(glVertexAttribPointer, 1, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, Normal));

		glCall(glEnableVertexAttribArray, 2);
		glCall(glVertexAttribPointer, 2, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void*)offsetof(Mesh::Vertex, TexCoords));

	}

	void MeshRenderer::draw(const Mesh& _mesh, const Texture2D& _texture, const glm::mat4& _transform) {
		meshes.push_back(_mesh);
		//textures.push_back(_texture);
		transforms.push_back(_transform);
		dirty = true;
	}

	void MeshRenderer::present(const Camera& _camera) {
		//ggf. Program

		if (dirty) {
			glCall(glBindBuffer, GL_ARRAY_BUFFER, vbo);
			glCall(glBufferData, GL_ARRAY_BUFFER, meshes[0].vertices.size() * sizeof(Mesh::Vertex),
				&(meshes[0].vertices[0]), GL_STATIC_DRAW); //meshes[0] zeichnet nur ein mesh
			dirty = false;
		}
		glCall(glBindVertexArray, vao);
		glCall(glDrawArrays, GL_TRIANGLES, 0, meshes[0].vertices.size() * 3);
	}

	void MeshRenderer::clear() {
		meshes.clear();
		//textures.clear();
		transforms.clear();
		dirty = true;
	}

}
