#pragma once

#include "SNBX/Defines.hpp"
#include "PlatformTypes.hpp"
#include "SNBX/Core/Math.hpp"

namespace SNBX::Platform
{
	SNBX_API void Init();
	SNBX_API void Shutdown();

	SNBX_API void       ProcessEvents();
	SNBX_API Window*    CreateWindow(const StringView& title, UVec2 size, WindowFlags flags);
	SNBX_API Extent     GetWindowExtent(Window* window);
	SNBX_API bool       UserRequestedClose(Window* window);
	SNBX_API void       DestroyWindow(Window* window);

	SNBX_API CPtr       LoadSharedLib(const StringView& libPath);
	SNBX_API CPtr       GetAddress(CPtr, const char* funcName);
}