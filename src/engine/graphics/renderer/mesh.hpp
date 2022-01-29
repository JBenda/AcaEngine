#pragma once

#include <engine/graphics/core/geometrybuffer.hpp>
#include "../../utils/meshloader.hpp"
#include <engine/utils/meshstripper.hpp>

namespace graphics {

    struct VertexData {
        glm::vec3 positionData;
        glm::vec2 uvData;
        glm::vec3 normalData;
    };

    class Mesh {
    public:
        Mesh(const utils::MeshData::Handle &_meshData);

        static constexpr std::array<graphics::VertexAttribute, 3> VERTEX_ATTRIBUTES = {
                graphics::VertexAttribute{
                        graphics::PrimitiveFormat::FLOAT,
                        3,
                        false,
                        false
                },
                graphics::VertexAttribute{
                        graphics::PrimitiveFormat::FLOAT,
                        2,
                        false,
                        false
                },
                graphics::VertexAttribute{
                        graphics::PrimitiveFormat::FLOAT,
                        3,
                        false,
                        false
                }
        };

        const std::vector<graphics::GeometryBuffer *> &getGeometryBuffers() { return buffers; }

    private:
        std::vector<graphics::GeometryBuffer *> buffers = {};

    };
}
