#include "app.hpp"
#include "common.hpp"
#include "device/gpu_device.hpp"
#include "sandbox/platform/platform.hpp"
#include "sandbox/assets/asset_server.hpp"

Window*         window = nullptr;
GPUSwapchain    swapchain = {};
bool            running = true;

i32 app_init() {
    spdlog::info("sandbox initialized");

    platform_init();
    asset_server_init();
    gpu_device_init();

    window = platform_create_window(WindowCreation{
            .title = "Sandbox",
            .maximized = true
    });

    swapchain = gpu_device_create_swapchain(SwapChainCreation{
        .vsync = true
    }, window);

    while (running) {

        platform_process_events();

        if (platform_window_request_close(window)) {
            running = false;
            break;
        }
    }

    gpu_device_destroy_swapchain(swapchain);
    gpu_device_shutdown();
    platform_destroy_window(window);

    return 0;
}
