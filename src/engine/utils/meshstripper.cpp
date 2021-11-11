#include <iostream>
#include "meshstripper.hpp"

namespace utils {
    TriangleStrip *TriangleStrip::create(utils::MeshData::FaceData face1, utils::MeshData::FaceData face2, std::array<int, 4> vertices) {
        auto *strip = new TriangleStrip;
        strip->triangles.push_back(face1);
        strip->triangles.push_back(face2);
        for (const auto &vertexIndex: vertices) {
            strip->vertexIndices.push_back(vertexIndex);
        }
        return strip;
    }

    TriangleStrip *
    TriangleStrip::create(utils::MeshData::FaceData face1, utils::MeshData::FaceData face2, utils::MeshData::FaceData face3, std::array<int, 5> vertices) {
        auto *strip = new TriangleStrip;
        strip->triangles.push_back(face1);
        strip->triangles.push_back(face2);
        strip->triangles.push_back(face3);
        for (const auto &vertexIndex: vertices) {
            strip->vertexIndices.push_back(vertexIndex);
        }
        return strip;
    }

    TriangleStrip::~TriangleStrip() {
        vertexIndices.clear();
        triangles.clear();
    }

    bool findConnectedTriangle(std::array<int, 3> firstFace, std::vector<utils::MeshData::FaceData> *remainingFaces, utils::MeshData::FaceData &face2,
                               std::array<int, 4> &result) {
        int connectedVertexIndex;
        for (auto iterator = remainingFaces->begin(); iterator != remainingFaces->end(); ++iterator) {
            connectedVertexIndex = 1; // reset index
            for (const auto &vertexB: (*iterator).indices) {
                bool didMatch = false;
                for (const auto &indexA: firstFace) {
                    if (indexA == vertexB.positionIdx) {
                        didMatch = true;
                        result[connectedVertexIndex] = indexA;
                        connectedVertexIndex++;
                    }
                }
                if (!didMatch) {
                    result[3] = vertexB.positionIdx;
                }
            }

            if (connectedVertexIndex > 2) {
                // find unique vertex of first face
                for (const auto &indexA: firstFace) {
                    if (indexA != result[1] && indexA != result[2]) {
                        result[0] = indexA;
                        break;
                    }
                }

                remainingFaces->erase(iterator);
                return true;
            }
        }
        return false;
    }

    bool findConnectedTriangle(std::array<int, 4> stripOfTwo, std::vector<utils::MeshData::FaceData> *remainingFaces, utils::MeshData::FaceData &face3,
                               std::array<int, 5> &result) {
        for (auto iterator = remainingFaces->begin(); iterator != remainingFaces->end(); ++iterator) {
            int outerMatchIndex = -1; // marks which end of the strip the triangle connects to
            int innerMatchIndex = -1; // marks which inner vertex the triangle connects to
            int uniqueVertex = 0;     // the one vertex of the triangle that doesn't connect to the strip
            for (const auto &vertexB: (*iterator).indices) {
                int vertexIndex = vertexB.positionIdx;
                if (vertexIndex == stripOfTwo[0]) {
                    outerMatchIndex = 0;
                } else if (vertexIndex == stripOfTwo[3]) {
                    outerMatchIndex = 3;
                } else if (vertexIndex == stripOfTwo[1]) {
                    innerMatchIndex = 1;
                } else if (vertexIndex == stripOfTwo[2]) {
                    innerMatchIndex = 2;
                } else {
                    uniqueVertex = vertexIndex;
                }
            }

            // only consider triangles that have an outer and inner match
            if (outerMatchIndex < 0 || innerMatchIndex < 0) {
                continue;
            }

            // make sure the inner and outer matching vertices are directly connected
            if ((outerMatchIndex == 0 && innerMatchIndex == 2)
                || (outerMatchIndex == 3 && innerMatchIndex == 1)) {
                int temp = stripOfTwo[2];
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

    bool areFacesConnected(int vertex1, int vertex2, utils::MeshData::FaceData face, int &foundVertex) {
        int matches = 0;
        for (const auto &index: face.indices) {
            if (index.positionIdx == vertex1 || index.positionIdx == vertex2) {
                matches++;
            } else {
                foundVertex = index.positionIdx;
            }
        }
        return matches >= 2;
    }

    void expandTriangleStrip(TriangleStrip *stripToExpand, std::vector<utils::MeshData::FaceData> *remainingFaces) {
        int vertA1 = stripToExpand->vertexIndices[0];
        int vertA2 = stripToExpand->vertexIndices[1];
        const auto vertIteratorEnd = stripToExpand->vertexIndices.end();
        int vertB1 = *(vertIteratorEnd - 1);
        int vertB2 = *(vertIteratorEnd - 2);

        int count = 0;
        while (!remainingFaces->empty()) {
            int foundConnectedFace = false;
            for (auto iterator = remainingFaces->begin(); iterator != remainingFaces->end(); ++iterator) {
                int foundVertex;
                if (areFacesConnected(vertA1, vertA2, *iterator, foundVertex)) {
                    vertA2 = vertA1;
                    vertA1 = foundVertex;
                    stripToExpand->vertexIndices.insert(stripToExpand->vertexIndices.begin(), foundVertex);
                    stripToExpand->triangles.insert(stripToExpand->triangles.begin(), *iterator);
                    remainingFaces->erase(iterator);
                    foundConnectedFace = true;
                    count++;
                    break;
                }
                if (areFacesConnected(vertB1, vertB2, *iterator, foundVertex)) {
                    vertB2 = vertB1;
                    vertB1 = foundVertex;
                    stripToExpand->vertexIndices.push_back(foundVertex);
                    stripToExpand->triangles.push_back(*iterator);
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

            utils::MeshData::FaceData face2;
            std::array<int, 4> twoTriangleStrip = {};
            bool foundFace2 = findConnectedTriangle(
                    {
                            face1.indices[0].positionIdx,
                            face1.indices[1].positionIdx,
                            face1.indices[2].positionIdx
                    },
                    &remainingFaces,
                    face2,
                    twoTriangleStrip
            );
            if (!foundFace2) {
                result->floatingTriangles.push_back(face1);
                continue;
            }

            utils::MeshData::FaceData face3;
            std::array<int, 5> threeTriangleStrip = {};
            bool foundFace3 = findConnectedTriangle(
                    twoTriangleStrip,
                    &remainingFaces,
                    face3,
                    threeTriangleStrip
            );

            if (!foundFace3) {
                result->triangleStrips.push_back(*TriangleStrip::create(face1, face2, twoTriangleStrip));
                continue;
            }

            TriangleStrip *strip = TriangleStrip::create(face1, face2, face3, threeTriangleStrip);
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