// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "Defines.hpp"
#include "SNBX/Platform/PlatformTypes.hpp"
#include "SNBX/Render/RenderTypes.hpp"

namespace SNBX
{
	struct AppCreation
	{
		String        title      = "SNBX";
		UVec2         size       = UVec2(1920, 1080);
		bool          fullscreen = false;
		bool          vsync      = true;
		GPUDeviceType device     = GPUDeviceType_OpenGL;
		Vec4          clearColor = Vec4{0, 0, 0, 1};
	};

	namespace App
	{
		SNBX_API void Init(const AppCreation& creation);
		SNBX_API bool Update();
		SNBX_API void Shutdown();
	}
}