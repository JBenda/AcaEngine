#pragma once

#include "../core/shader.hpp"
#include "../camera.hpp"
#include "glm/glm.hpp"
#include <vector>

namespace graphics {

	class Mesh;
	class Texture2D;

	class MeshRenderer
	{
	public:
		MeshRenderer() {}

		void draw(const Mesh& _mesh, const Texture2D& _texture, const glm::mat4& _transform) {}

		void present(const Camera& _camera) {}
		void clear() {}
	private:
	};
}