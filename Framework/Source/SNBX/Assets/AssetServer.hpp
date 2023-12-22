// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#pragma once

#include "SNBX/Defines.hpp"
#include "AssetTypes.hpp"

namespace SNBX::AssetServer
{
	SNBX_API void Init();
	SNBX_API void AddAssetHandler(AssetHandler assetHandler, StringView extensions);
	SNBX_API void Shutdown();
}