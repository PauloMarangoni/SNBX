#include "app.hpp"
#include "common.hpp"
#include "sandbox/platform/platform.hpp"
#include "sandbox/assets/asset_server.hpp"

Window *window = nullptr;
bool running = true;

i32 app_init() {
    spdlog::info("sandbox initialized");

    platform_init();
    asset_server_init();

    window = platform_create_window(WindowCreation{
            .title = "Sandbox",
            .maximized = true
    });

    while (running) {

        platform_process_events();

        if (platform_window_request_close(window)) {
            running = false;
            break;
        }
    }

    platform_destroy_window(window);

    return 0;
}
