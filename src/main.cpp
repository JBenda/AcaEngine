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

    static graphics::Sampler sampler(graphics::Sampler::Filter::LINEAR, graphics::Sampler::Filter::LINEAR,
                                     graphics::Sampler::Filter::LINEAR, graphics::Sampler::Border::MIRROR);
    auto texture = graphics::Texture2DManager::get("textures/planet1.png", sampler);
    texture->bind(0);
    utils::MeshData::Handle data = utils::MeshLoader::get("models/sphere.obj");
    utils::MeshStripData::Handle meshStripData = utils::MeshStripData::createFromMeshData(data);

    graphics::VertexAttribute vertexAttribute[] = {
            {
                    graphics::PrimitiveFormat::FLOAT,
                    3,
                    false,
                    false
            },
            {
                    graphics::PrimitiveFormat::FLOAT,
                    2,
                    false,
                    false
            }
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

    std::vector < graphics::GeometryBuffer * > buffers = {};

    {
        graphics::GeometryBuffer *buffer = createStripBuffer(vertexAttribute, 1);
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
            graphics::GeometryBuffer *buffer = createStripBuffer(vertexAttribute, 1);
            buffer->setData(&triangles_tri, sizeof(triangles_tri));
            buffers.push_back(buffer);
        }
        if (input::InputManager::isKeyPressed(input::Key::R)) {
            buffers.clear();
            graphics::GeometryBuffer *buffer = createStripBuffer(vertexAttribute, 1);
            buffer->setData(&triangles_rect, sizeof(triangles_rect));
            buffers.push_back(buffer);
        }
        if (input::InputManager::isKeyPressed(input::Key::S)) {
            if (!wasSPressed) {
                wasSPressed = true;
                buffers.clear();

                graphics::GeometryBuffer *triBuffer = createTriangleBuffer(vertexAttribute, 2);
                {
                    auto *triBufferData = new VertexData[meshStripData->floatingTriangles.size() * 3];
                    int dataIndex = 0;
                    for (const auto &floatingTriangle: meshStripData->floatingTriangles) {
                        auto tex0 = data->textureCoordinates.at(floatingTriangle.indices[0].textureCoordinateIdx.value());
                        auto tex1 = data->textureCoordinates.at(floatingTriangle.indices[1].textureCoordinateIdx.value());
                        auto tex2 = data->textureCoordinates.at(floatingTriangle.indices[2].textureCoordinateIdx.value());
                        
                        std::cout << "uv coord 0 is " << tex0.x << "/" << tex0.y << " for vertex " << floatingTriangle.indices[0].positionIdx << std::endl;
                        std::cout << "uv coord 1 is " << tex1.x << "/" << tex1.y << " for vertex " << floatingTriangle.indices[1].positionIdx << std::endl;
                        std::cout << "uv coord 2 is " << tex2.x << "/" << tex2.y << " for vertex " << floatingTriangle.indices[2].positionIdx << std::endl;
                        triBufferData[dataIndex] = {
                                data->positions[floatingTriangle.indices[0].positionIdx],
                                data->textureCoordinates.at(floatingTriangle.indices[0].textureCoordinateIdx.value())
                        };

                        triBufferData[dataIndex] = {
                                data->positions[floatingTriangle.indices[1].positionIdx],
                                data->textureCoordinates.at(floatingTriangle.indices[1].textureCoordinateIdx.value())
                        };

                        triBufferData[dataIndex] = {
                                data->positions[floatingTriangle.indices[2].positionIdx],
                                data->textureCoordinates.at(floatingTriangle.indices[2].textureCoordinateIdx.value())
                        };
                        
                        dataIndex += 3;
                    }
                    triBuffer->setData(triBufferData, sizeof(glm::vec3) * meshStripData->floatingTriangles.size() * 3);
                }
                buffers.push_back(triBuffer);

                /*for (const auto &strip: meshStripData->triangleStrips) {
                    auto *stripBuffer = createStripBuffer(vertexAttribute, 1);
                    unsigned int vertexCount = strip.vertexIndices.size();
                    auto *stripBufferData = new glm::vec3[vertexCount];
                    int dataIndex = 0;
                    for (const auto &vertexIndex: strip.vertexIndices) {
                        stripBufferData[dataIndex] = data->positions[vertexIndex];
                        dataIndex++;
                    }
                    stripBuffer->setData(stripBufferData, sizeof(glm::vec3) * vertexCount);
                    buffers.push_back(stripBuffer);
                }*/
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