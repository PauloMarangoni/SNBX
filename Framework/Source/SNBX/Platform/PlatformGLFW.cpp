#include "Platform.hpp"

#ifdef SNBX_DESKTOP

#include "SNBX/Core/Logger.hpp"
#include <GLFW/glfw3.h>

namespace SNBX
{

	struct Window
	{
		GLFWwindow* handler;
	};

	void Platform::Init()
	{
		if (!glfwInit())
		{
			Log::Error("Error on initialize GLFW");
			return;
		}

		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	}

	void Platform::InitOpenGL()
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef SNBX_APPLE
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	}

	void Platform::Shutdown()
	{
		glfwTerminate();
	}

	Window* Platform::CreateWindow(const StringView& title, UVec2 size, WindowFlags flags)
	{
		bool maximized = (flags & WindowFlags_Maximized);
		glfwWindowHint(GLFW_MAXIMIZED, maximized);

		float xScale = 1.f, yScale = 1.f;
		glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &xScale, &yScale);
		size = {u32(size.x * xScale), u32(size.y * yScale)};

		Window* window = new Window{};
		window->handler = glfwCreateWindow(size.x, size.y, title.data(), nullptr, nullptr);

		glfwShowWindow(window->handler);

		return window;
	}

	void Platform::ProcessEvents()
	{
		glfwPollEvents();
	}

	bool Platform::UserRequestedClose(Window* window)
	{
		return glfwWindowShouldClose(window->handler);
	}

	void Platform::DestroyWindow(Window* window)
	{
		glfwDestroyWindow(window->handler);
		delete window;
	}

}

#endif