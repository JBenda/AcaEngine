#pragma once

#include <glm/glm.hpp>

namespace graphics {

    class Camera {
    public:
        // Perspective
        // @param _fov as angle
        Camera(float _fov, float _zNear, float zFar);

        // Orthogonal
        Camera(glm::vec2 _size, glm::vec2 _origin = glm::vec2(0.f), float _zNear = 0.f, float _zFar = 1.f);

        [[nodiscard]] const glm::mat4 &getView() const { return m_view; }

        [[nodiscard]] const glm::mat4 &getProjection() const { return m_projection; }

        [[nodiscard]] const glm::mat4 &getViewProjection() const { return m_viewProjection; }

        void setView(const glm::mat4 &_view) {
            m_view = _view;
            updateMatrices();
        }

        // @Return the position of the point _screenSpace on the near plane of this camera
        [[nodiscard]] glm::vec3 toWorldSpace(const glm::vec2 &_screenSpace) const;

        [[nodiscard]] const glm::vec3 &getPosition() const { return cameraPosition; }

        void setPosition(const glm::vec3 &position) {
            cameraPosition = position;
            updateTransform();
        }
        
        void moveRelative(const float &right, const float &up, const float &forward);
        
        void moveAbsolute(const glm::vec3 &direction);

        [[nodiscard]] const glm::vec3 &forwardVector() const { return cameraForward; }

        [[nodiscard]] const glm::vec3 &upVector() const { return cameraUp; }

        [[nodiscard]] const glm::vec3 &rightVector() const { return cameraRight; }
        
        // sets the camera rotation
        void setRotation(const float &yawAngle, const float &pitchAngle, const float &rollAngle);

        // sets the camera rotation
        void setRotation(const glm::mat3 &rotationMatrix);
        
        // rotates the camera based on its current rotation
        void rotate(const float &yawAngle, const float &pitchAngle, const float &rollAngle);

        // rotates the camera based on its current rotation
        void rotate(const glm::mat3 &rotationMatrix);

        [[nodiscard]] const glm::mat4 &getWorldToCamera() const { return worldToCameraMatrix; }

    private:
        void updateMatrices();

        void updateTransform();

        glm::mat4 m_projection;
        glm::mat4 m_view;
        glm::mat4 m_viewProjection;
        glm::mat4 m_viewProjectionInv;

        glm::vec3 cameraPosition = {0.0f, 0.0f, 0.0f};
        glm::vec3 cameraForward = {0.0f, 0.0f, 1.0f};
        glm::vec3 cameraUp = {0.0f, 1.0f, 0.0f};
        glm::vec3 cameraRight = {1.0f, 0.0f, 0.0f};
        glm::mat4 lookAtMatrix;
        
        glm::mat4 worldToCameraMatrix;
    };
}