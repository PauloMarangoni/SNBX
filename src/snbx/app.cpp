#include "app.hpp"
#include "common.hpp"
#include "snbx/device/gpu_device.hpp"
#include "snbx/platform/platform.hpp"
#include "snbx/assets/asset_server.hpp"

Window*         window = nullptr;
GPUSwapchain    swapchain = {};
bool            running = true;

void app_init() {
    spdlog::info("SNBX initialized");

    platform_init();
    asset_server_init();
    gpu_device_init();

    window = platform_create_window(WindowCreation{
            .title = "Sandbox",
            .maximized = true
    });

    swapchain = gpu_device_create_swapchain(SwapchainCreation{
        .vsync = true
    }, window);
}

bool app_update() {
    platform_process_events();
    if (platform_window_request_close(window)) {
        running = false;
        return false;
    }
    return running;
}

void app_destroy() {
    gpu_device_destroy_swapchain(swapchain);
    gpu_device_shutdown();
    platform_destroy_window(window);
}
