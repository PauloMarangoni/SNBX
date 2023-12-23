// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#include "SNBX/Assets/AssetServer.hpp"
#include "SNBX/Render/RenderAssets.hpp"

namespace SNBX::ShaderAssetHandler
{
	void CompileShader(const StringView& id, const Path& assetPath)
	{
		int a= 0;
	}
}

namespace SNBX
{
	void RegisterShaderAssetHandler()
	{
		AssetHandler assetHandler = {
			.CompileAsset = ShaderAssetHandler::CompileShader
		};
		AssetServer::AddAssetHandler(assetHandler, ".hlsl", GetTypeId<ShaderAsset>());
	}
}