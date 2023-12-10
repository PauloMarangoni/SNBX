#include "asset_server.hpp"

Path asset_server_asset_path;
Path asset_server_data_path;

void asset_server_init() {
    auto current = std::filesystem::current_path();
    auto assets = current / "assets";

    while (!std::filesystem::exists(assets)) {
        auto parent = current.parent_path();
        if (!std::filesystem::exists(parent)) {
            SNBX_ASSERT(false, "asset folder not found");
            return;
        }
        current = parent;
        assets = current / "assets";
    }

    asset_server_asset_path = assets;
    asset_server_data_path = current / "data";

    spdlog::info("[AssetServer] asset folder found {} ", asset_server_asset_path.string());
    spdlog::info("[AssetServer] data folder found {} ", asset_server_data_path.string());
}

Path asset_server_get_asset_path() {
    return asset_server_asset_path;
}
