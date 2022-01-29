#include "camera.hpp"
#include "core/device.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace graphics {

    using namespace glm;
    constexpr glm::vec3 RIGHT_VECTOR = glm::vec3(1.0f, 0.0f, 0.0f);
    constexpr glm::vec3 UP_VECTOR = glm::vec3(0.0f, 1.0f, 0.0f);
    constexpr glm::vec3 FORWARD_VECTOR = glm::vec3(0.0f, 0.0f, 1.0f);

    Camera::Camera(float _fov, float _zNear, float zFar)
            : m_projection(glm::perspective(glm::radians(_fov), Device::getAspectRatio(), _zNear, zFar)),
              m_view(glm::identity<glm::mat4>()) {
        updateMatrices();
    }

    Camera::Camera(glm::vec2 _size, glm::vec2 _origin, float _zNear, float _zFar)
            : m_projection(glm::ortho(-_size.x * _origin.x, _size.x * (1.f - _origin.x),
                                      -_size.y * _origin.y, _size.y * (1.f - _origin.y),
                                      _zNear, _zFar)),
              m_view(glm::identity<glm::mat4>()) {
        updateMatrices();
    }

    vec3 Camera::toWorldSpace(const vec2 &_screenSpace) const {
        vec2 clipSpace = _screenSpace / vec2(Device::getBufferSize());
        clipSpace.y = 1.f - clipSpace.y;
        clipSpace = clipSpace * 2.f - vec2(1.f, 1.f);

        const vec4 worldSpace = m_viewProjectionInv * vec4(clipSpace, -1.f, 1.f);

        return vec3(worldSpace) / worldSpace.w;
    }

    void Camera::updateMatrices() {
        m_viewProjection = m_projection * m_view;
        m_viewProjectionInv = glm::inverse(m_viewProjection);
        updateTransform();
    }

    void Camera::updateTransform() {
        lookAtMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);
        // fix x direction (gets inverted due to RHS / cross product)
        lookAtMatrix[0][0] *= -1.0f;
        lookAtMatrix[1][0] *= -1.0f;
        lookAtMatrix[2][0] *= -1.0f;
        lookAtMatrix[3][0] *= -1.0f;
        worldToCameraMatrix = m_viewProjection * lookAtMatrix;
    }

    void Camera::moveRelative(const float &right, const float &up, const float &forward) {
        if (right == 0.0f && up == 0.0f && forward == 0.0f) {
            return;
        }

        if (right != 0.0f) {
            cameraPosition += cameraRight * right;
        }
        if (up != 0.0f) {
            cameraPosition += cameraUp * up;
        }
        if (forward != 0.0f) {
            cameraPosition += cameraForward * forward;
        }
        updateTransform();
    }

    void Camera::moveAbsolute(const vec3 &direction) {
        cameraPosition += direction;
        updateTransform();
    }

    void Camera::setRotation(const float &yawAngle, const float &pitchAngle, const float &rollAngle) {
        cameraRight = RIGHT_VECTOR;
        cameraUp = UP_VECTOR;
        cameraForward = FORWARD_VECTOR;
        rotate(yawAngle, pitchAngle, rollAngle);
    }

    void Camera::setRotation(const glm::mat3 &rotationMatrix) {
        cameraRight = rotationMatrix * RIGHT_VECTOR;
        cameraUp = rotationMatrix * UP_VECTOR;
        cameraForward = rotationMatrix * FORWARD_VECTOR;
        updateTransform();
    }

    void Camera::rotate(const float &yawAngle, const float &pitchAngle, const float &rollAngle) {
        if (yawAngle == 0.0f && pitchAngle == 0.0f && rollAngle == 0.0f) {
            return;
        }

        if (yawAngle != 0.0f) {
            auto yawRotationMatrix = glm::mat3(glm::rotate(
                    glm::identity<glm::mat4>(),
                    glm::radians(yawAngle),
                    cameraUp));
            cameraRight = yawRotationMatrix * cameraRight;
            cameraForward = yawRotationMatrix * cameraForward;
        }
        if (pitchAngle != 0.0f) {
            auto pitchRotationMatrix = glm::mat3(glm::rotate(
                    glm::identity<glm::mat4>(),
                    glm::radians(pitchAngle),
                    cameraRight));
            cameraForward = pitchRotationMatrix * cameraForward;
            cameraUp = pitchRotationMatrix * cameraUp;
        }
        if (rollAngle != 0.0f) {
            auto rollRotationMatrix = glm::mat3(glm::rotate(
                    glm::identity<glm::mat4>(),
                    glm::radians(rollAngle),
                    cameraForward));
            cameraRight = rollRotationMatrix * cameraRight;
            cameraUp = rollRotationMatrix * cameraUp;
        }

        updateTransform();
    }

    void Camera::rotate(const mat3 &rotationMatrix) {
        cameraRight = rotationMatrix * cameraRight;
        cameraUp = rotationMatrix * cameraUp;
        cameraForward = rotationMatrix * cameraForward;
        updateTransform();
    }

}