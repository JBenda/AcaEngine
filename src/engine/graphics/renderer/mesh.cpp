#include "mesh.hpp"

namespace graphics {

    static graphics::GeometryBuffer *createStripBuffer(const graphics::VertexAttribute *vertexAttribute, int numAttributes) {
        auto *buffer = new graphics::GeometryBuffer(
                graphics::GLPrimitiveType::TRIANGLE_STRIPE,
                vertexAttribute,
                numAttributes,
                0
        );
        return buffer;
    }

    static graphics::GeometryBuffer *createTriangleBuffer(const graphics::VertexAttribute *vertexAttribute, int numAttributes) {
        auto *buffer = new graphics::GeometryBuffer(
                graphics::GLPrimitiveType::TRIANGLES,
                vertexAttribute,
                numAttributes,
                0
        );
        return buffer;
    }

    Mesh::Mesh(const utils::MeshData::Handle &_meshData) {
        utils::MeshStripData::Handle meshStripData = utils::MeshStripData::createFromMeshData(_meshData);
        graphics::GeometryBuffer *triBuffer = createTriangleBuffer(VERTEX_ATTRIBUTES.data(), VERTEX_ATTRIBUTES.size());
        auto *triBufferData = new VertexData[meshStripData->floatingTriangles.size() * 3];
        int dataIndex = 0;
        for (const auto &floatingTriangle: meshStripData->floatingTriangles) {
            for (int i = 0; i < 3; i++) {
                triBufferData[dataIndex + i] = {
                        _meshData->positions[floatingTriangle.indices[i].positionIdx],
                        _meshData->textureCoordinates[floatingTriangle.indices[i].textureCoordinateIdx.value_or(0)],
                        _meshData->normals[floatingTriangle.indices[i].normalIdx.value_or(0)]
                };
            }
            dataIndex += 3;
        }
        triBuffer->setData(triBufferData, sizeof(VertexData) * meshStripData->floatingTriangles.size() * 3);
        buffers.push_back(triBuffer);

        for (const auto &strip: meshStripData->triangleStrips) {
            auto *stripBuffer = createStripBuffer(VERTEX_ATTRIBUTES.data(), VERTEX_ATTRIBUTES.size());
            unsigned int vertexCount = strip.vertexIndices.size();
            auto *stripBufferData = new VertexData[vertexCount];
            dataIndex = 0;
            for (const auto &vertexIndex: strip.vertexIndices) {
                stripBufferData[dataIndex] = {
                        _meshData->positions[vertexIndex.positionIdx],
                        _meshData->textureCoordinates[vertexIndex.textureCoordinateIdx.value_or(0)],
                        _meshData->normals[vertexIndex.normalIdx.value_or(0)]
                };
                dataIndex++;
            }
            stripBuffer->setData(stripBufferData, sizeof(VertexData) * vertexCount);
            buffers.push_back(stripBuffer);
        }
    }
}