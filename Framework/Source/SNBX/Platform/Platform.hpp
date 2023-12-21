#pragma once

#include "SNBX/Defines.hpp"
#include "PlatformTypes.hpp"

namespace SNBX::Platform
{
	SNBX_API void Init();
	SNBX_API void Shutdown();

	SNBX_API void ProcessEvents();
	SNBX_API Window* CreateWindow(const StringView& title, UVec2 size, WindowFlags flags);
	SNBX_API bool UserRequestedClose(Window* window);
	SNBX_API void DestroyWindow(Window* window);

	//OpenGL
	SNBX_API void InitOpenGL();

}