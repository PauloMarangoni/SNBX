// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#include "AssetServer.hpp"
#include "SNBX/Defines.hpp"
#include "SNBX/Core/Logger.hpp"

namespace SNBX
{
	struct AssetStorage
	{

	};

	struct AssetContext
	{
		Path                                     assetPath;
		Path                                     dataPath;
		HashMap<String, SharedPtr<AssetStorage>> assets;
		HashMap<String, AssetHandler>            assetHandlers;
	};

	AssetContext assetContext{};

	void LoadAsset(const String& path, const Path& assetPath)
	{
		auto it = assetContext.assetHandlers.find(assetPath.extension().string());
		if (it != assetContext.assetHandlers.end())
		{
			if (it->second.CompileAsset)
			{
				it->second.CompileAsset(assetPath);
			}
		}
	}

	void ScanFolder(const String& path, const Path& assetPath)
	{
		for (const DirEntry& dirEntry: DirIterator(assetPath))
		{
			String assetPath = path + "/" + dirEntry.path().filename().string();
			if (dirEntry.is_directory())
			{
				ScanFolder(assetPath, dirEntry.path());
			}
			else
			{
				LoadAsset(assetPath, dirEntry.path());
			}
		}
	}

	void AssetServer::Init()
	{
		Path current = std::filesystem::current_path();
		Path assets  = current / "Assets";

		while (!std::filesystem::exists(assets))
		{
			auto parent = current.parent_path();
			if (!std::filesystem::exists(parent))
			{
				SNBX_ASSERT(false, "Asset folder not found");
				return;
			}
			current = parent;
			assets  = current / "Assets";
		}
		assetContext.assetPath = assets;
		assetContext.dataPath  = std::filesystem::current_path() / "Data";

		Log::Debug("Asset Folder {}", assetContext.assetPath.string());
		Log::Debug("Data Folder {}", assetContext.dataPath.string());

		for (const DirEntry& dirEntry: DirIterator(assetContext.assetPath))
		{
			if (dirEntry.is_directory())
			{
				ScanFolder(dirEntry.path().filename().string() + ":/", dirEntry.path());
			}
		}
	}

	void AssetServer::AddAssetHandler(AssetHandler assetHandler, StringView extensions)
	{
		assetContext.assetHandlers.emplace(extensions, assetHandler);
	}

	void AssetServer::Shutdown()
	{

	}
}