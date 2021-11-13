#include <iostream>
#include "meshstripper.hpp"

using VertexIndices = utils::MeshData::FaceData::VertexIndices;
namespace utils {
    TriangleStrip *TriangleStrip::create(std::array<VertexIndices, 4> vertices) {
        auto *strip = new TriangleStrip;
        for (const auto &vertexIndex: vertices) {
            strip->vertexIndices.push_back(vertexIndex);
        }
        return strip;
    }

    TriangleStrip *
    TriangleStrip::create(std::array<VertexIndices, 5> vertices) {
        auto *strip = new TriangleStrip;
        for (const auto &vertexIndex: vertices) {
            strip->vertexIndices.push_back(vertexIndex);
        }
        return strip;
    }

    TriangleStrip::~TriangleStrip() {
        vertexIndices.clear();
    }

    bool findConnectedTriangle(utils::MeshData::FaceData *firstFace, std::vector<utils::MeshData::FaceData> *remainingFaces,
                               std::array<VertexIndices, 4> &result) {
        int connectedVertexIndex;
        for (auto iterator = remainingFaces->begin(); iterator != remainingFaces->end(); ++iterator) {
            connectedVertexIndex = 1; // reset index
            for (const auto &vertexB: (*iterator).indices) {
                bool didMatch = false;
                for (const auto &vertexA: firstFace->indices) {
                    if (vertexA.positionIdx == vertexB.positionIdx) {
                        didMatch = true;
                        result[connectedVertexIndex] = vertexA;
                        connectedVertexIndex++;
                    }
                }
                if (!didMatch) {
                    result[3] = vertexB;
                }
            }

            if (connectedVertexIndex > 2) {
                // find unique vertex of first face
                for (const auto &vertexA: firstFace->indices) {
                    if (vertexA.positionIdx != result[1].positionIdx && vertexA.positionIdx != result[2].positionIdx) {
                        result[0] = vertexA;
                        break;
                    }
                }

                remainingFaces->erase(iterator);
                return true;
            }
        }
        return false;
    }

    bool findConnectedTriangle(std::array<VertexIndices, 4> stripOfTwo, std::vector<utils::MeshData::FaceData> *remainingFaces,
                               std::array<VertexIndices, 5> &result) {
        for (auto iterator = remainingFaces->begin(); iterator != remainingFaces->end(); ++iterator) {
            int outerMatchIndex = -1; // marks which end of the strip the triangle connects to
            int innerMatchIndex = -1; // marks which inner vertex the triangle connects to
            VertexIndices uniqueVertex;     // the one vertex of the triangle that doesn't connect to the strip
            for (const auto &vertexB: (*iterator).indices) {
                int vertexIndex = vertexB.positionIdx;
                if (vertexIndex == stripOfTwo[0].positionIdx) {
                    outerMatchIndex = 0;
                } else if (vertexIndex == stripOfTwo[3].positionIdx) {
                    outerMatchIndex = 3;
                } else if (vertexIndex == stripOfTwo[1].positionIdx) {
                    innerMatchIndex = 1;
                } else if (vertexIndex == stripOfTwo[2].positionIdx) {
                    innerMatchIndex = 2;
                } else {
                    uniqueVertex = vertexB;
                }
            }

            // only consider triangles that have an outer and inner match
            if (outerMatchIndex < 0 || innerMatchIndex < 0) {
                continue;
            }

            // make sure the inner and outer matching vertices are directly connected
            if ((outerMatchIndex == 0 && innerMatchIndex == 2)
                || (outerMatchIndex == 3 && innerMatchIndex == 1)) {
                VertexIndices temp = stripOfTwo[2];
                stripOfTwo[2] = stripOfTwo[1];
                stripOfTwo[1] = temp;
            }

            if (outerMatchIndex == 0) {
                result[0] = uniqueVertex;
                std::copy(stripOfTwo.begin(), stripOfTwo.begin() + 4, result.begin() + 1);
            } else if (outerMatchIndex == 3) {
                std::copy(stripOfTwo.begin(), stripOfTwo.begin() + 4, result.begin());
                result[4] = uniqueVertex;
            }
            remainingFaces->erase(iterator);
            return true;
        }
        return false;
    }

    bool areFacesConnected(VertexIndices vertex1, VertexIndices vertex2, utils::MeshData::FaceData face, VertexIndices &foundVertex) {
        int matches = 0;
        for (const auto &index: face.indices) {
            if (index.positionIdx == vertex1.positionIdx || index.positionIdx == vertex2.positionIdx) {
                matches++;
            } else {
                foundVertex = index;
            }
        }
        return matches >= 2;
    }

    void expandTriangleStrip(TriangleStrip *stripToExpand, std::vector<utils::MeshData::FaceData> *remainingFaces) {
        VertexIndices vertA1 = stripToExpand->vertexIndices[0];
        VertexIndices vertA2 = stripToExpand->vertexIndices[1];
        const auto vertIteratorEnd = stripToExpand->vertexIndices.end();
        VertexIndices vertB1 = *(vertIteratorEnd - 1);
        VertexIndices vertB2 = *(vertIteratorEnd - 2);

        int count = 0;
        while (!remainingFaces->empty()) {
            int foundConnectedFace = false;
            for (auto iterator = remainingFaces->begin(); iterator != remainingFaces->end(); ++iterator) {
                VertexIndices foundVertex;
                if (areFacesConnected(vertA1, vertA2, *iterator, foundVertex)) {
                    vertA2 = vertA1;
                    vertA1 = foundVertex;
                    stripToExpand->vertexIndices.insert(stripToExpand->vertexIndices.begin(), foundVertex);
                    remainingFaces->erase(iterator);
                    foundConnectedFace = true;
                    count++;
                    break;
                }
                if (areFacesConnected(vertB1, vertB2, *iterator, foundVertex)) {
                    vertB2 = vertB1;
                    vertB1 = foundVertex;
                    stripToExpand->vertexIndices.push_back(foundVertex);
                    remainingFaces->erase(iterator);
                    foundConnectedFace = true;
                    count++;
                    break;
                }
            }

            if (!foundConnectedFace) {
                break;
            }
        }
    }

    MeshStripData::Handle MeshStripData::createFromMeshData(const utils::MeshData *data) {
        auto *result = new MeshStripData;

        std::vector<utils::MeshData::FaceData> remainingFaces = data->faces;
        while (!remainingFaces.empty()) {
            utils::MeshData::FaceData face1 = *remainingFaces.begin();
            remainingFaces.erase(remainingFaces.begin());

            std::array<VertexIndices, 4> twoTriangleStrip = {};
            bool foundFace2 = findConnectedTriangle(
                    &face1,
                    &remainingFaces,
                    twoTriangleStrip
            );
            if (!foundFace2) {
                result->floatingTriangles.push_back(face1);
                continue;
            }

            std::array<VertexIndices, 5> threeTriangleStrip = {};
            bool foundFace3 = findConnectedTriangle(
                    twoTriangleStrip,
                    &remainingFaces,
                    threeTriangleStrip
            );

            if (!foundFace3) {
                result->triangleStrips.push_back(*TriangleStrip::create(twoTriangleStrip));
                continue;
            }

            TriangleStrip *strip = TriangleStrip::create(threeTriangleStrip);
            expandTriangleStrip(strip, &remainingFaces);
            result->triangleStrips.push_back(*strip);
        }

        return result;
    }

    MeshStripData::~MeshStripData() {
        triangleStrips.clear();
        floatingTriangles.clear();
    }
}