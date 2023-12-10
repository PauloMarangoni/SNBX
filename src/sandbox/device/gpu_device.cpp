#include "gpu_device.hpp"
#include "gpu_device_api.hpp"

GPUDeviceAPI device_api{};

void d3d12_device_register(GPUDeviceAPI& gpu_device_api);
void vulkan_device_register(GPUDeviceAPI& gpu_device_api);

void gpu_device_init() {
   // d3d12_device_register(device_api);
    vulkan_device_register(device_api);
    device_api.init();
}

GPUSwapchain gpu_device_create_swapchain(const SwapChainCreation& swapchain_creation, Window* window) {
    return device_api.create_swapchain(swapchain_creation, window);
}

void gpu_device_destroy_swapchain(const GPUSwapchain& swapchain) {
    device_api.destroy_swapchain(swapchain);
}

void gpu_device_shutdown() {
    device_api.shutdown();
}
