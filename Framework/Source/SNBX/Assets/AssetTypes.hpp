// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "SNBX/Defines.hpp"

namespace SNBX
{
	struct AssetHandler
	{
		void (*CompileAsset)(const Path& assetPath);
		void (*LoadAsset)(const Path& assetPath, CPtr instance);
		void (*DestroyAsset)(CPtr instance);
	};
}