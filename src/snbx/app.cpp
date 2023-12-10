#include "app.hpp"
#include "common.hpp"
#include "shader/shader_compiler.hpp"
#include "snbx/device/gpu_device.hpp"
#include "snbx/platform/platform.hpp"
#include "snbx/assets/asset_server.hpp"

Window*         window = nullptr;
GPUSwapchain    swapchain = {};
bool            running = true;

void app_init() {
    spdlog::info("SNBX initialized");

    platform_init();
    shader_compiler_init();
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

    UVec2 swapchain_extent = platform_window_get_size(window);

    GPUCommands cmd = gpu_device_begin_frame();

    Vec4 clear_value = Vec4(0, 0, 0, 1);

    gpu_device_begin_render_pass(cmd, BeginRenderPassInfo{
        .extent = swapchain_extent,
        .swapchain = swapchain,
        .clear_values = {&clear_value, 1}
    });

    ViewportInfo viewport_info{};
    viewport_info.x = 0.;
    viewport_info.y = 0.;
    viewport_info.width = (f32) swapchain_extent.x;
    viewport_info.height = (f32) swapchain_extent.y;
    viewport_info.max_depth = 0.;
    viewport_info.min_depth = 1.;
    gpu_device_set_viewport(cmd, viewport_info);

    Rect rect = {0, 0, swapchain_extent.x, swapchain_extent.y};
    gpu_device_set_scissor(cmd, rect);

    gpu_device_end_render_pass(cmd);

    gpu_device_end_frame(swapchain);

    return running;
}

void app_destroy() {
    gpu_device_wait();
    gpu_device_destroy_swapchain(swapchain);
    gpu_device_shutdown();
    platform_destroy_window(window);
}
