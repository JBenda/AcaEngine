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

#endif
#endif

using namespace std::chrono_literals;

graphics::GeometryBuffer *createStripBuffer(graphics::VertexAttribute *vertexAttribute) {
    auto *buffer = new graphics::GeometryBuffer(
            graphics::GLPrimitiveType::TRIANGLE_STRIPE,
            vertexAttribute,
            1,
            0
    );
    return buffer;
}

graphics::GeometryBuffer *createTriangleBuffer(graphics::VertexAttribute *vertexAttribute) {
    auto *buffer = new graphics::GeometryBuffer(
            graphics::GLPrimitiveType::TRIANGLES,
            vertexAttribute,
            1,
            0
    );
    return buffer;
}

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

    utils::MeshData::Handle data = utils::MeshLoader::get("models/sphere.obj");
    utils::MeshStripData::Handle meshStripData = utils::MeshStripData::createFromMeshData(data);

    graphics::VertexAttribute vertexAttribute{
            graphics::PrimitiveFormat::FLOAT,
            3,
            false,
            false
    };

    glm::vec3 triangles_tri[] = {
            glm::vec3(-1.0f, -1.0f, 0.0f),
            glm::vec3(1.0f, -1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
    };
    glm::vec3 triangles_rect[] = {
            glm::vec3(-0.75f, -0.75f, 0.0f),
            glm::vec3(0.75f, -0.75f, 0.0f),
            glm::vec3(-0.75f, 0.75f, 0.0f),
            glm::vec3(0.75f, 0.75f, 0.0f),
    };

    std::vector<graphics::GeometryBuffer *> buffers = {};

    {
        graphics::GeometryBuffer *buffer = createStripBuffer(&vertexAttribute);
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

    graphics::glCall(glClearColor, 0.f, 1.f, 0.f, 1.f);

    bool wasSPressed = false;

    while (!glfwWindowShouldClose(window) && !input::InputManager::isKeyPressed(input::Key::ESCAPE)) {
        graphics::glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (input::InputManager::isKeyPressed(input::Key::T)) {
            buffers.clear();
            graphics::GeometryBuffer *buffer = createStripBuffer(&vertexAttribute);
            buffer->setData(&triangles_tri, sizeof(triangles_tri));
            buffers.push_back(buffer);
        }
        if (input::InputManager::isKeyPressed(input::Key::R)) {
            buffers.clear();
            graphics::GeometryBuffer *buffer = createStripBuffer(&vertexAttribute);
            buffer->setData(&triangles_rect, sizeof(triangles_rect));
            buffers.push_back(buffer);
        }
        if (input::InputManager::isKeyPressed(input::Key::S)) {
            if (!wasSPressed) {
                wasSPressed = true;
                buffers.clear();

                graphics::GeometryBuffer *triBuffer = createTriangleBuffer(&vertexAttribute);
                {
                    auto *triBufferData = new glm::vec3[meshStripData->floatingTriangles.size() * 3];
                    int dataIndex = 0;
                    for (const auto &floatingTriangle: meshStripData->floatingTriangles) {
                        triBufferData[dataIndex] = data->positions[floatingTriangle.indices[0].positionIdx];
                        triBufferData[dataIndex + 1] = data->positions[floatingTriangle.indices[1].positionIdx];
                        triBufferData[dataIndex + 2] = data->positions[floatingTriangle.indices[2].positionIdx];
                        dataIndex += 3;
                    }
                    triBuffer->setData(triBufferData, sizeof(glm::vec3) * meshStripData->floatingTriangles.size() * 3);
                }
                buffers.push_back(triBuffer);

                for (const auto &strip: meshStripData->triangleStrips) {
                    auto *stripBuffer = createStripBuffer(&vertexAttribute);
                    unsigned int vertexCount = strip.vertexIndices.size();
                    auto *stripBufferData = new glm::vec3[vertexCount];
                    int dataIndex = 0;
                    for (const auto &vertexIndex: strip.vertexIndices) {
                        stripBufferData[dataIndex] = data->positions[vertexIndex];
                        dataIndex++;
                    }
                    stripBuffer->setData(stripBufferData, sizeof(glm::vec3) * vertexCount);
                    buffers.push_back(stripBuffer);
                }
            }
        } else {
            wasSPressed = false;
        }

        for (const auto &buffer: buffers) {
            buffer->draw();
        }

        glfwPollEvents();
        glfwSwapBuffers(window);
        std::this_thread::sleep_for(16ms);
    }

    utils::MeshLoader::clear();
    graphics::Device::close();
    return EXIT_SUCCESS;
}