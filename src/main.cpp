#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/graphics/core/opengl.hpp>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <engine/utils/meshstripper.hpp>
#include <engine/graphics/core/geometrybuffer.hpp>
#include <engine/graphics/resources.hpp>

#include <thread>

// CRT's memory leak detection
#ifndef NDEBUG
#if defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#endif
#endif

using namespace std::chrono_literals;

graphics::GeometryBuffer *createStripBuffer(graphics::VertexAttribute *vertexAttribute, int numAttributes) {
    auto *buffer = new graphics::GeometryBuffer(
            graphics::GLPrimitiveType::TRIANGLE_STRIPE,
            vertexAttribute,
            numAttributes,
            0
    );
    return buffer;
}

graphics::GeometryBuffer *createTriangleBuffer(graphics::VertexAttribute *vertexAttribute, int numAttributes) {
    auto *buffer = new graphics::GeometryBuffer(
            graphics::GLPrimitiveType::TRIANGLES,
            vertexAttribute,
            numAttributes,
            0
    );
    return buffer;
}

struct VertexData {
    glm::vec3 positionData;
    glm::vec2 uvData;
    glm::vec3 normalData;
};

struct LightData {
    inline static const int LIGHT_TYPE_DIRECTIONAL = 0;
    inline static const int LIGHT_TYPE_SPOT = 1;
    inline static const int LIGHT_TYPE_POINT = 2;

    /**
     * Directional Light:
     *   position     -
     *   direction    direction the light is emitted in
     *   range        -
     *   spot_angle   -
     *   color        color of the emitted light
     *   intensity    simple multiplier for emitted light
     * Spot-Light:
     *   position     position of the light source in world space
     *   direction    direction the spot-light is facing
     *   range        maximum range of emitted light
     *   spot_angle   max angle the light is emitted at in degrees (e.g. 5° means light is emitted in a cone of 2.5° from the spot-light's direction)
     *   color        color of the emitted light
     *   intensity    resulting light-intensity at a given point is `intensity * ( 1 - ([point_to_light_distance] / [range]) )²`
     * Point-Light:
     *   position     position of the light source in world space
     *   direction    -
     *   range        maximum range of emitted light
     *   spot_angle   -
     *   color        color of the emitted light
     *   intensity    resulting light-intensity at a given point is `intensity * ( 1 - ([point_to_light_distance] / [range]) )²`
     */

    int light_type;
    glm::vec3 light_position;
    glm::vec3 light_direction;
    float light_range;
    float light_spot_angle;
    glm::vec3 light_color;
    float light_intensity;
};

int main(int argc, char *argv[]) {
#ifndef NDEBUG
#if defined(_MSC_VER)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //	_CrtSetBreakAlloc(2760);
#endif
#endif

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    graphics::Device::initialize(1366, 1366, false);
    GLFWwindow *window = graphics::Device::getWindow();
    input::InputManager::initialize(window);
    input::InputManager::setCursorMode(input::InputManager::CursorMode::NORMAL);

    glm::vec3 ambientLightData = {0.3f, 0.3f, 0.3f};
    // TODO create light type specific constructors
    LightData lightData = {
            LightData::LIGHT_TYPE_DIRECTIONAL,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f)),
            0.0f, 0.0f,
            glm::vec3(1.0f, 1.0f, 1.0f),
            1.0f
    };

    graphics::Camera camera(90.0f, 0.1f, 300.0f);
    static graphics::Sampler sampler(graphics::Sampler::Filter::LINEAR, graphics::Sampler::Filter::LINEAR,
                                     graphics::Sampler::Filter::LINEAR, graphics::Sampler::Border::MIRROR);
    auto planetTexture = graphics::Texture2DManager::get("textures/planet1.png", sampler);
    auto planetPhong = graphics::Texture2DManager::get("textures/Planet1_phong.png", sampler);
    planetTexture->bind(0);
    planetPhong->bind(1);
    utils::MeshData::Handle data = utils::MeshLoader::get("models/sphere.obj");
    utils::MeshStripData::Handle meshStripData = utils::MeshStripData::createFromMeshData(data);

    std::array<graphics::VertexAttribute, 3> vertexAttributes = {
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

    VertexData triangles_tri[] = {
            {glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(-1.0f, -1.0f)},
            {glm::vec3(1.0f, -1.0f, 0.0f),  glm::vec2(1.0f, -1.0f)},
            {glm::vec3(0.0f, 1.0f, 0.0f),   glm::vec2(0.0f, 1.0f)},
    };
    VertexData triangles_rect[] = {
            {glm::vec3(-0.75f, -0.75f, 0.0f), glm::vec2(0.0f, -0.75f)},
            {glm::vec3(0.75f, -0.75f, 0.0f),  glm::vec2(0.75f, -0.75f)},
            {glm::vec3(-0.75f, 0.75f, 0.0f),  glm::vec2(-0.75f, 0.75f)},
            {glm::vec3(0.75f, 0.75f, 0.0f),   glm::vec2(0.75f, 0.75f)},
    };

    std::vector<graphics::GeometryBuffer *> buffers = {};

    {
        graphics::GeometryBuffer *buffer = createStripBuffer(vertexAttributes.data(), vertexAttributes.size());
        buffer->setData(&triangles_rect, sizeof(triangles_rect));
        buffers.push_back(buffer);
    }

    graphics::Shader::Handle fragmentShader = graphics::ShaderManager::get("shader/demo.frag", graphics::ShaderType::FRAGMENT);
    graphics::Shader::Handle vertexShader = graphics::ShaderManager::get("shader/demo.vert", graphics::ShaderType::VERTEX);

    graphics::Program program;
    program.attach(vertexShader);
    program.attach(fragmentShader);
    program.link();
    program.use();

    const glm::vec4 rightVector = {1.0f, 0.0f, 0.0f, 0.0f};
    const glm::vec4 forwardVector = {0.0f, 0.0f, 1.0f, 0.0f};
    const glm::vec4 upVector = {0.0f, 1.0f, 0.0f, 0.0f};
    glm::vec4 cameraPosition = {0.0f, 0.0f, -3.0f, 0.0f};
    glm::vec4 cameraForwardVector = forwardVector;
    glm::vec4 cameraUpVector = upVector;
    glm::vec4 cameraRightVector = rightVector;
    const float cameraStep = 0.05f;
    const float cameraPitchSensitivity = 0.5f;
    const float cameraYawSensitivity = 0.5f;
    const float cameraRollStep = 0.05f;
    float cameraPitch = 0.0f;
    float cameraYaw = 0.0f;
    float cameraRoll = 0.0f;
    glm::mat4 worldToCameraMatrix;
    GLint worldToCameraMatrixID = glGetUniformLocation(program.getID(), "world_to_camera_matrix");

    GLint cameraPositionShaderID = glGetUniformLocation(program.getID(), "camera_position");
    GLint glsl_ambient_light = glGetUniformLocation(program.getID(), "ambient_light");

    // TODO create a function for binding lightData
    GLint glsl_light_type = glGetUniformLocation(program.getID(), "light_type");
    GLint glsl_light_position = glGetUniformLocation(program.getID(), "light_position");
    GLint glsl_light_direction = glGetUniformLocation(program.getID(), "light_direction");
    GLint glsl_light_range = glGetUniformLocation(program.getID(), "light_range");
    GLint glsl_light_spot_angle = glGetUniformLocation(program.getID(), "light_spot_angle");
    GLint glsl_light_color = glGetUniformLocation(program.getID(), "light_color");
    GLint glsl_light_intensity = glGetUniformLocation(program.getID(), "light_intensity");

    graphics::glCall(glEnable, GL_DEPTH_TEST);
    graphics::glCall(glClearColor, 0.05f, 0.0f, 0.05f, 1.f);

    glm::vec2 prevCursorPos;
    glm::vec2 currentCursorPos = input::InputManager::getCursorPos();

    while (!glfwWindowShouldClose(window) && !input::InputManager::isKeyPressed(input::Key::ESCAPE)) {
        prevCursorPos = currentCursorPos;
        currentCursorPos = input::InputManager::getCursorPos();
        graphics::glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (input::InputManager::isKeyPressed(input::Key::T)) {
            buffers.clear();
            graphics::GeometryBuffer *buffer = createStripBuffer(vertexAttributes.data(), vertexAttributes.size());
            buffer->setData(&triangles_tri, sizeof(triangles_tri));
            buffers.push_back(buffer);
        }
        if (input::InputManager::isKeyPressed(input::Key::R)) {
            buffers.clear();
            graphics::GeometryBuffer *buffer = createStripBuffer(vertexAttributes.data(), vertexAttributes.size());
            buffer->setData(&triangles_rect, sizeof(triangles_rect));
            buffers.push_back(buffer);
        }
        if (input::InputManager::isKeyPressed(input::Key::S)) {
            /*std::cout << "floating triangles: " << meshStripData->floatingTriangles.size() << std::endl;
            std::cout << "triangle strips: " << meshStripData->triangleStrips.size() << std::endl;
            for (const auto &triStrip: meshStripData->triangleStrips) {
                std::cout << "  vertices: " << triStrip.vertexIndices.size() << std::endl;
            }*/
            buffers.clear();

            if (input::InputManager::isKeyPressed(input::Key::Num1)) {
                graphics::GeometryBuffer *triBuffer = createTriangleBuffer(vertexAttributes.data(), vertexAttributes.size());
                auto *triBufferData = new VertexData[meshStripData->floatingTriangles.size() * 3];
                int dataIndex = 0;
                for (const auto &floatingTriangle: meshStripData->floatingTriangles) {
                    for (int i = 0; i < 3; i++) {
                        triBufferData[dataIndex + i] = {
                                data->positions[floatingTriangle.indices[i].positionIdx],
                                data->textureCoordinates[floatingTriangle.indices[i].textureCoordinateIdx.value_or(0)],
                                data->normals[floatingTriangle.indices[i].normalIdx.value_or(0)]
                        };
                    }
                    dataIndex += 3;
                }
                triBuffer->setData(triBufferData, sizeof(VertexData) * meshStripData->floatingTriangles.size() * 3);
                buffers.push_back(triBuffer);
            }

            if (input::InputManager::isKeyPressed(input::Key::Num3)) {
                for (const auto &strip: meshStripData->triangleStrips) {
                    auto *stripBuffer = createStripBuffer(vertexAttributes.data(), vertexAttributes.size());
                    unsigned int vertexCount = strip.vertexIndices.size();
                    auto *stripBufferData = new VertexData[vertexCount];
                    int dataIndex = 0;
                    for (const auto &vertexIndex: strip.vertexIndices) {
                        stripBufferData[dataIndex] = {
                                data->positions[vertexIndex.positionIdx],
                                data->textureCoordinates[vertexIndex.textureCoordinateIdx.value_or(0)],
                                data->normals[vertexIndex.normalIdx.value_or(0)]
                        };
                        dataIndex++;
                    }
                    stripBuffer->setData(stripBufferData, sizeof(VertexData) * vertexCount);
                    buffers.push_back(stripBuffer);
                }
            }
        }

        if (input::InputManager::isKeyPressed(input::Key::KP_8)) {
            cameraPosition += cameraForwardVector * cameraStep;
            std::cout << "camera position: " << cameraPosition[0] << " / " << cameraPosition[1] << " / " << cameraPosition[2] << std::endl;
        }
        if (input::InputManager::isKeyPressed(input::Key::KP_2)) {
            cameraPosition -= cameraForwardVector * cameraStep;
            std::cout << "camera position: " << cameraPosition[0] << " / " << cameraPosition[1] << " / " << cameraPosition[2] << std::endl;
        }
        if (input::InputManager::isKeyPressed(input::Key::KP_6)) {
            cameraPosition += cameraRightVector * cameraStep;
            std::cout << "camera position: " << cameraPosition[0] << " / " << cameraPosition[1] << " / " << cameraPosition[2] << std::endl;
        }
        if (input::InputManager::isKeyPressed(input::Key::KP_4)) {
            cameraPosition -= cameraRightVector * cameraStep;
            std::cout << "camera position: " << cameraPosition[0] << " / " << cameraPosition[1] << " / " << cameraPosition[2] << std::endl;
        }
        if (input::InputManager::isKeyPressed(input::Key::KP_9)) {
            cameraPosition += cameraUpVector * cameraStep;
            std::cout << "camera position: " << cameraPosition[0] << " / " << cameraPosition[1] << " / " << cameraPosition[2] << std::endl;
        }
        if (input::InputManager::isKeyPressed(input::Key::KP_3)) {
            cameraPosition -= cameraUpVector * cameraStep;
            std::cout << "camera position: " << cameraPosition[0] << " / " << cameraPosition[1] << " / " << cameraPosition[2] << std::endl;
        }

        float cursorDeltaX = currentCursorPos.x - prevCursorPos.x;
        float cursorDeltaY = currentCursorPos.y - prevCursorPos.y;
        if (cursorDeltaX != 0.0f) {
            cameraYaw += cursorDeltaX * cameraYawSensitivity;
            if (cameraYaw > 180.0f) {
                cameraYaw -= 360.0f;
            } else if (cameraYaw < -180.0f) {
                cameraYaw += 360.0f;
            }
        }
        if (cursorDeltaY != 0.0f) {
            cameraPitch += cursorDeltaY * cameraPitchSensitivity;
            if (cameraPitch > 90.0f) {
                cameraPitch = 90.0f;
            } else if (cameraPitch < -90.0f) {
                cameraPitch = -90.0f;
            }
        }

        // update camera up/forward/right vectors according to camera rotation
        // apply yaw, then pitch, then roll
        auto identityMat4 = glm::identity<glm::mat4>();
        auto yawRotationMatrix = glm::rotate(identityMat4, glm::radians(cameraYaw), glm::vec3(cameraUpVector));
        cameraRightVector = yawRotationMatrix * rightVector;
        auto pitchRotationMatrix = glm::rotate(identityMat4, glm::radians(cameraPitch), glm::vec3(cameraRightVector));
        cameraForwardVector = pitchRotationMatrix * yawRotationMatrix * forwardVector;
        auto rollRotationMatrix = glm::rotate(identityMat4, glm::radians(cameraRoll), glm::vec3(cameraForwardVector));
        cameraRightVector = rollRotationMatrix * cameraRightVector;
        cameraUpVector = rollRotationMatrix * pitchRotationMatrix * upVector;
        
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));

        // TODO create a function for binding lightData
        glUniform1i(glsl_light_type, lightData.light_type);
        glUniform3fv(glsl_light_position, 1, glm::value_ptr(lightData.light_position));
        glUniform3fv(glsl_light_direction, 1, glm::value_ptr(lightData.light_direction));
        glUniform1f(glsl_light_range, lightData.light_range);
        glUniform1f(glsl_light_spot_angle, lightData.light_spot_angle);
        glUniform3fv(glsl_light_color, 1, glm::value_ptr(lightData.light_color));
        glUniform1f(glsl_light_intensity, lightData.light_intensity);

        auto lookAtMatrix = glm::lookAt(glm::vec3(cameraPosition), glm::vec3(cameraPosition + cameraForwardVector), glm::vec3(cameraUpVector));
        // fix x direction
        lookAtMatrix[0][0] *= -1.0f;
        lookAtMatrix[1][0] *= -1.0f;
        lookAtMatrix[2][0] *= -1.0f;
        lookAtMatrix[3][0] *= -1.0f;
        worldToCameraMatrix = camera.getViewProjection() * lookAtMatrix;
        glUniformMatrix4fv(worldToCameraMatrixID, 1, GL_FALSE, glm::value_ptr(worldToCameraMatrix));
        glUniform3fv(cameraPositionShaderID, 1, glm::value_ptr(cameraPosition));
        for (const auto &buffer: buffers) {
            buffer->draw();
        }

        glfwPollEvents();
        glfwSwapBuffers(window);
        graphics::glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        std::this_thread::sleep_for(16ms);
    }

    utils::MeshLoader::clear();
    graphics::Device::close();
    return EXIT_SUCCESS;
}