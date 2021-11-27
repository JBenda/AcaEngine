#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/graphics/core/opengl.hpp>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <engine/utils/meshstripper.hpp>
#include <engine/graphics/lightdata.h>
#include <engine/graphics/core/geometrybuffer.hpp>
#include <engine/graphics/resources.hpp>

#include <thread>

// CRT's memory leak detection
#ifndef NDEBUG
#if defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>
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

    glm::vec3 ambientLightData = {0.7f, 0.7f, 0.7f};
    graphics::LightData lightData = graphics::LightData::directional(
            glm::normalize(glm::vec3(-1.0f, -1.0f, 0.5f)),
            glm::vec3(1.0f, 1.0f, 1.0f),
            2.0f);

    graphics::Camera camera(90.0f, 0.1f, 300.0f);
    camera.setPosition(glm::vec3(0.0f, 0.0f, -3.0f));
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
            {glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(-1.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
            {glm::vec3(1.0f, -1.0f, 0.0f),  glm::vec2(1.0f, -1.0f),  glm::vec3(0.0f, 0.0f, -1.0f)},
            {glm::vec3(0.0f, 1.0f, 0.0f),   glm::vec2(0.0f, 1.0f),   glm::vec3(0.0f, 0.0f, -1.0f)},
    };
    VertexData triangles_rect[] = {
            {glm::vec3(-0.75f, -0.75f, 0.0f), glm::vec2(0.0f, -0.75f),  glm::vec3(0.0f, 0.0f, -1.0f)},
            {glm::vec3(0.75f, -0.75f, 0.0f),  glm::vec2(0.75f, -0.75f), glm::vec3(0.0f, 0.0f, -1.0f)},
            {glm::vec3(-0.75f, 0.75f, 0.0f),  glm::vec2(-0.75f, 0.75f), glm::vec3(0.0f, 0.0f, -1.0f)},
            {glm::vec3(0.75f, 0.75f, 0.0f),   glm::vec2(0.75f, 0.75f),  glm::vec3(0.0f, 0.0f, -1.0f)},
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
    
    lightData.bindData(program.getID());

    const float cameraStep = 0.05f;
    const float cameraPitchSensitivity = 0.5f;
    const float cameraYawSensitivity = 0.5f;
    const float cameraRollStep = 0.05f;
    float cameraPitch = 0.0f;
    float cameraYaw = 0.0f;
    float cameraRoll = 0.0f;
    GLint worldToCameraMatrixID = glGetUniformLocation(program.getID(), "world_to_camera_matrix");

    GLint cameraPositionShaderID = glGetUniformLocation(program.getID(), "camera_position");
    GLint glsl_ambient_light = glGetUniformLocation(program.getID(), "ambient_light");

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

        float rightInput = 0.0f;
        float forwardInput = 0.0f;
        float upInput = 0.0f;
        if (input::InputManager::isKeyPressed(input::Key::KP_8)) {
            forwardInput += cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::KP_2)) {
            forwardInput -= cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::KP_6)) {
            rightInput += cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::KP_4)) {
            rightInput -= cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::KP_9)) {
            upInput += cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::KP_3)) {
            upInput -= cameraStep;
        }
        camera.moveRelative(rightInput, upInput, forwardInput);

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
        camera.setRotation(cameraYaw, cameraPitch, cameraRoll);

        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
        glUniformMatrix4fv(worldToCameraMatrixID, 1, GL_FALSE, glm::value_ptr(camera.getWorldToCamera()));
        glUniform3fv(cameraPositionShaderID, 1, glm::value_ptr(camera.getPosition()));
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