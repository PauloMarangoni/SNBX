// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "SNBX/Defines.hpp"

#define RENDER_HANDLER(StructName) struct StructName { \
CPtr handler;                                      \
 operator bool() const {return handler != nullptr; }          \
 bool operator==(const StructName& b) const { return this->handler == b.handler; } \
 bool operator!=(const StructName& b) const { return this->handler != b.handler; } \
}

namespace SNBX
{
	RENDER_HANDLER(RenderSwapchain);
	RENDER_HANDLER(RenderPass);
	RENDER_HANDLER(RenderCommands);

	enum GPUDeviceType_
	{
		GPUDeviceType_None   = 0,
		GPUDeviceType_Vulkan = 1,
		GPUDeviceType_OpenGL = 2,
		GPUDeviceType_D3D12  = 2,
		GPUDeviceType_Metal   = 3
	};

	typedef u32 GPUDeviceType;

	struct BeginRenderPassInfo
	{
		RenderPass      renderPass{};
		RenderSwapchain swapchain{};
		Span<Vec4>      clearValues{};
		Vec2            depthStencil{1.0, 0.0};
	};

	struct ViewportInfo
	{
		f32 x{};
		f32 y{};
		f32 width{};
		f32 height{};
		f32 minDepth{};
		f32 maxDepth{};
	};
}