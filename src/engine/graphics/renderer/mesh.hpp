#pragma once

#include "../../utils/meshLoader.hpp"

namespace graphics {


	class Mesh
	{
	public:
		struct Vertex {
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec2 TexCoords;
		};

		Mesh(const utils::MeshData& _meshData);

		std::vector<Vertex> vertices;
	};
}
