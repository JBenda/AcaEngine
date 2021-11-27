#include "meshrenderer.hpp"
#include "../core/texture.hpp"

namespace graphics {


    void MeshRenderer::draw(const Mesh &_mesh, const Texture2D &_texture, const Texture2D &phongData, const glm::mat4 &_transform) {
        auto *renderData = new MeshRenderData{
                _mesh,
                _texture,
                phongData,
                _transform
        };
        meshBuffer.push_back(renderData);
    }

    void MeshRenderer::present(const unsigned int &programID) {
        if (currentProgramID != programID){
            currentProgramID = programID;
            glsl_object_to_world_matrix = glGetUniformLocation(programID, "object_to_world_matrix");
        }
        
        for (const auto &meshRenderData: meshBuffer){
            meshRenderData->textureData.bind(0);
            meshRenderData->phongData.bind(1);
            glUniformMatrix4fv(glsl_object_to_world_matrix, 1, false, glm::value_ptr(meshRenderData->transform));
            for (const auto &geometryBuffer : meshRenderData->meshData.getGeometryBuffers()){
                geometryBuffer->draw();
            }
        }
    }

    void MeshRenderer::clear() {
        meshBuffer.clear();
    }
}
