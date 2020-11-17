#include "mesh.hpp"
#include "mesh.hpp"

namespace graphics {

	Mesh::Mesh(const utils::MeshData& _meshData) {
		for (auto it = _meshData.faces.begin(); it != _meshData.faces.end(); ++it) { //TODO: value_or weg
			struct Vertex v1 = { _meshData.positions[it->indices[0].positionIdx], _meshData.normals[it->indices[0].normalIdx.value_or(0)],
				_meshData.textureCoordinates[it->indices[0].textureCoordinateIdx.value_or(0)] };
			struct Vertex v2 = { _meshData.positions[it->indices[1].positionIdx], _meshData.normals[it->indices[1].normalIdx.value_or(0)],
				_meshData.textureCoordinates[it->indices[1].textureCoordinateIdx.value_or(0)] };
			struct Vertex v3 = { _meshData.positions[it->indices[2].positionIdx], _meshData.normals[it->indices[2].normalIdx.value_or(0)],
				_meshData.textureCoordinates[it->indices[2].textureCoordinateIdx.value_or(0)] };
			vertices.push_back(v1);
			vertices.push_back(v2);
			vertices.push_back(v3);
		}
	}

}
