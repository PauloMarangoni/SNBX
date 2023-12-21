#include "Platform.hpp"

#ifdef SNBX_DESKTOP

#include "SNBX/Core/Logger.hpp"
#include <GLFW/glfw3.h>

namespace SNBX
{
	void Platform::Init()
	{
		if (!glfwInit())
		{
			Log::Error("Error on initialize GLFW");
			return;
		}
	}

	void Platform::Shutdown()
	{
		glfwTerminate();
	}
}

#endif