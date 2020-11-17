#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/utils/meshloader.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/utils/meshloader.hpp>
#include <engine\graphics\core\opengl.hpp>
#include <engine\graphics\core\geometrybuffer.hpp>
#include <gl/GL.h>
#include <GLFW/glfw3.h>

#include <thread>

// CRT's memory leak detection
#ifndef NDEBUG 
#if defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <engine\graphics\core\vertexformat.hpp>
#endif
#include <engine\graphics\core\texture.hpp>

using namespace std::chrono_literals;

int main(int argc, char* argv[])
{
#ifndef NDEBUG 
#if defined(_MSC_VER)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//	_CrtSetBreakAlloc(2760);
#endif
#endif

	graphics::Device::initialize(1366, 768, false);
	GLFWwindow* window = graphics::Device::getWindow();
	input::InputManager::initialize(window);


	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
	};
	struct graphics::VertexAttribute va[] = { {graphics::PrimitiveFormat::FLOAT, 3, false, false}};
	graphics::GeometryBuffer gb(graphics::GLPrimitiveType::TRIANGLES, va, 1, 0, 1024);
	gb.setData(g_vertex_buffer_data, sizeof(g_vertex_buffer_data));

	using namespace graphics;
	Camera camera(45.f, 0.01f, 10000.f);
	Sampler sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR, Sampler::Filter::LINEAR);
	const Texture2D& texture = *Texture2DManager::get("textures\\planet1.png", sampler);
	Mesh mesh(*utils::MeshLoader::get("models\\sphere.obj"));
	MeshRenderer renderer;
	glm::mat4 rot(1.f);



	glClearColor(0.f, 1.f, 0.f, 1.f);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderer.draw(mesh, texture, rot);
		renderer.present(camera);
		//gb.draw();


		glfwPollEvents();
		glfwSwapBuffers(window);
		std::this_thread::sleep_for(16ms);
	}


	//graphics::glCall(glDeleteVertexArrays, 1, &vertexArrayID);


	utils::MeshLoader::clear();
	graphics::Device::close();
	return EXIT_SUCCESS;
}
