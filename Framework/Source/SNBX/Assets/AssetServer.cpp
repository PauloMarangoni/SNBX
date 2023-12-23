// Copyright 2023 Paulo Marangoni.
// Use of this source code is governed by the license that can be found in the LICENSE file at the root of this distribution.

#include "AssetServer.hpp"
#include "SNBX/Defines.hpp"
#include "SNBX/Core/Logger.hpp"

namespace SNBX
{
	struct AssetStorage
	{
		String id;
	};

	struct AssetContext
	{
		Path                                     assetPath;
		Path                                     dataPath;
		HashMap<String, SharedPtr<AssetStorage>> assets;
		HashMap<String, AssetHandler>            assetHandlers;
		HashMap<TypeID , AssetHandler>           assetHandlersById;
	};

	AssetContext assetContext{};

	void LoadAsset(AssetStorage& parent, const String& path, const Path& assetPath)
	{
		auto it = assetContext.assets.find(path);
		if (it == assetContext.assets.end())
		{
			String id = parent.id + "_" + assetPath.filename().string();
			for (char & i : id)
			{
				if (i == '.')
				{
					i = '_';
				}
			}
			it = assetContext.assets.emplace(path, std::make_shared<AssetStorage>(id)).first;
		}

		auto itHandler = assetContext.assetHandlers.find(assetPath.extension().string());
		if (itHandler != assetContext.assetHandlers.end())
		{
			if (itHandler->second.CompileAsset)
			{
				itHandler->second.CompileAsset(it->second->id, assetPath);
			}
		}
	}

	void ScanFolder(AssetStorage& parent, const String& path, const Path& assetPath)
	{
		for (const DirEntry& dirEntry: DirIterator(assetPath))
		{
			String assetPathId = path + "/" + dirEntry.path().filename().string();
			if (dirEntry.is_directory())
			{
				ScanFolder(parent, assetPathId, dirEntry.path());
			}
			else
			{
				LoadAsset(parent, assetPathId, dirEntry.path());
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
			AssetStorage parent{
				.id = dirEntry.path().filename().string()
			};

			if (dirEntry.is_directory())
			{
				ScanFolder(parent, dirEntry.path().filename().string() + ":/", dirEntry.path());
			}
		}
	}

	void AssetServer::AddAssetHandler(AssetHandler assetHandler, StringView extensions, TypeID typeId)
	{
		assetContext.assetHandlers.emplace(extensions, assetHandler);
		assetContext.assetHandlersById.emplace(typeId, assetHandler);
	}

	void AssetServer::Shutdown()
	{

	}
}