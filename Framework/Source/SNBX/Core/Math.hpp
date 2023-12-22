// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "SNBX/Defines.hpp"

namespace SNBX
{

	struct Extent
	{
		u32 width;
		u32 height;
	};


	struct Rect
	{
		i32 x;
		i32 y;
		u32 width;
		u32 height;
	};


	struct FRect
	{
		f32 x;
		f32 y;
		f32 width;
		f32 height;
	};
}