// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "SNBX/Defines.hpp"
#include "SNBX/App.hpp"
#include "SNBX/Core/Math.hpp"

namespace SNBX::RenderDevice
{
	SNBX_API void Init(const AppCreation& creation);
	SNBX_API void Shutdown();

	SNBX_API RenderSwapchain    CreateSwapchain(Window* window, bool vsync);
	SNBX_API void               DestroySwapchain(RenderSwapchain swapchain);

	SNBX_API void               BeginRenderPass(RenderCommands cmd, const BeginRenderPassInfo& beginRenderPassInfo);
	SNBX_API void               EndRenderPass(RenderCommands cmd);
	SNBX_API void               SetViewport(RenderCommands cmd, const ViewportInfo& viewportInfo);
	SNBX_API void               SetScissor(RenderCommands cmd, const Rect& rect);

	SNBX_API RenderCommands     BeginFrame();
	SNBX_API void               EndFrame(RenderSwapchain swapchain);

}
