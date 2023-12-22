// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "SNBX/Defines.hpp"

namespace SNBX::Platform
{
	SNBX_API void InitOpenGL();
	SNBX_API void SetVSync(bool vsync);
	SNBX_API void MakeContextCurrent(Window* window);
	SNBX_API void SwapBuffers(Window* window);
	SNBX_API CPtr GetProcAddress();
}