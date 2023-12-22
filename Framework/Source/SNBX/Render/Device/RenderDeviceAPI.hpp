// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "SNBX/Render/RenderTypes.hpp"
#include "SNBX/Platform/PlatformTypes.hpp"
#include "SNBX/Core/Math.hpp"

namespace SNBX
{
	struct RenderDeviceAPI
	{
		void (*Init)();
		void (*Shutdown)();

		RenderSwapchain (*CreateSwapchain)(Window* window, bool vsync);
		void            (*DestroySwapchain)(RenderSwapchain swapchain);

		void            (*BeginRenderPass)(RenderCommands cmd, const BeginRenderPassInfo& beginRenderPassInfo);
		void            (*EndRenderPass)(RenderCommands cmd);
		void            (*SetViewport)(RenderCommands cmd, const ViewportInfo& viewportInfo);
		void            (*SetScissor)(RenderCommands cmd, const Rect& rect);

		RenderCommands  (*BeginFrame)();
		void            (*EndFrame)(RenderSwapchain swapchain);
	};
}