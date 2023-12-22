// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.


#include "SNBX/Assets/AssetServer.hpp"
namespace SNBX::ShaderAssetHandler
{
	void CompileShader(const Path& assetPath)
	{
		int a = 0;
	}
}

namespace SNBX
{
	void RegisterShaderAssetHandler()
	{
		AssetServer::AddAssetHandler(AssetHandler{
			.CompileAsset = ShaderAssetHandler::CompileShader
		}, ".hlsl");
	}
}