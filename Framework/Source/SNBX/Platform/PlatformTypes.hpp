// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "SNBX/Defines.hpp"

namespace SNBX
{
	struct Window;

	enum WindowFlags_
	{
		WindowFlags_None          = 0,
		WindowFlags_Maximized     = 1 << 0
	};

	using WindowFlags = i32;
}