// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "Defines.hpp"
#include "SNBX/Platform/PlatformTypes.hpp"
#include "SNBX/Render/RenderTypes.hpp"

namespace SNBX
{
	typedef void(*FnOnRender)(RenderCommands renderCommands);
	typedef void(*FnOnUpdate)(f64 deltaTime);
	typedef void(*FnOnBlit)(RenderCommands renderCommands);

	struct AppCreation
	{
		String           Title      = "SNBX";
		UVec2            Size       = UVec2(1920, 1080);
		bool             Fullscreen = false;
		bool             Vsync      = true;
		RenderDeviceType Device     = RenderDeviceType_OpenGL;
		Vec4             ClearColor = Vec4{0, 0, 0, 1};
		FnOnRender       OnRender   = nullptr;
		FnOnUpdate       OnUpdate   = nullptr;
		FnOnBlit         OnBlit     = nullptr;
	};

	namespace App
	{
		SNBX_API void               Init(const AppCreation& creation);
		SNBX_API RenderSwapchain    GetSwapchain();
		SNBX_API Window*            GetWindow();
		SNBX_API bool               IsRunning();
		SNBX_API bool               Update();
		SNBX_API void               Shutdown();
	}
}