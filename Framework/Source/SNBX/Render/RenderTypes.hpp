// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "SNBX/Defines.hpp"

namespace SNBX
{
	enum GPUDeviceType_
	{
		GPUDeviceType_None   = 0,
		GPUDeviceType_Vulkan = 1,
		GPUDeviceType_OpenGL = 2,
		GPUDeviceType_D3D12  = 2,
		GPUDeviceType_Meta   = 3
	};

	typedef u32 GPUDeviceType;
}