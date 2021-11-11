#pragma once

#include <array>
#include <vector>
#include <engine/utils/meshloader.hpp>

namespace utils {

    /**
     * @brief describes a single triangle strip of a mesh
     */
    struct TriangleStrip {
        std::vector<int> vertexIndices = {};
        std::vector<utils::MeshData::FaceData> triangles = {};

        virtual ~TriangleStrip();

        static TriangleStrip* create(utils::MeshData::FaceData face1, utils::MeshData::FaceData face2, std::array<int, 4> vertices);

        static TriangleStrip*
        create(utils::MeshData::FaceData face1, utils::MeshData::FaceData face2, utils::MeshData::FaceData face3, std::array<int, 5> vertices);
    };

    /**
     * @brief contains the MeshData split into triangle strips
     */
    struct MeshStripData {
        std::vector<utils::MeshData::FaceData> floatingTriangles = {};
        std::vector<TriangleStrip> triangleStrips = {};

        virtual ~MeshStripData();

        using Handle = const MeshStripData *;

        static Handle createFromMeshData(const utils::MeshData *data);
    };
}
