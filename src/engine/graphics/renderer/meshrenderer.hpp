#pragma once

#include "../core/shader.hpp"
#include "../camera.hpp"
#include "glm/glm.hpp"
#include "mesh.hpp"
#include <vector>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

namespace graphics {

    class Texture2D;

    class MeshRenderer {
    public:
        MeshRenderer() = default;

        void draw(const Mesh &_mesh, const Texture2D &_texture, const Texture2D &phongData, const glm::mat4 &_transform);

        void present(const unsigned int &programID);

        void clear();

    private:
        unsigned int currentProgramID = -1;
        GLint glsl_object_to_world_matrix = 0;
        
        struct MeshRenderData{
            Mesh meshData;
            const Texture2D &textureData;
            const Texture2D &phongData;
            const glm::mat4 &transform;
        };
        
        std::vector<MeshRenderData *> meshBuffer = {};

    };
}