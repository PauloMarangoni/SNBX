// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "SNBX/Defines.hpp"
#include "RenderTypes.hpp"

namespace SNBX
{
	struct ShaderAsset
	{
		Array<u8>       shaderData{};
		ShaderBinTarget shaderBinTarget = {};
	};
}