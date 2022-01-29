﻿#pragma once

#include <array>
#include <vector>
#include <engine/utils/meshloader.hpp>

namespace utils {

    /**
     * @brief describes a single triangle strip of a mesh
     */
    struct TriangleStrip {
        std::vector<utils::MeshData::FaceData::VertexIndices> vertexIndices = {};

        virtual ~TriangleStrip();

        static TriangleStrip* create(std::array<utils::MeshData::FaceData::VertexIndices, 4> vertices);

        static TriangleStrip*
        create(std::array<utils::MeshData::FaceData::VertexIndices, 5> vertices);
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
