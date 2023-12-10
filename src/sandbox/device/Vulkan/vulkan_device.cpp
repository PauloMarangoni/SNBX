#include "sandbox/device/gpu_device_api.hpp"

namespace vk {

    void init() {

    }

    GPUSwapchain create_swapchain(const SwapChainCreation& swap_chain_creation, Window* window) {
        return {};
    }

    void destroy_swapchain(const GPUSwapchain& swapchain) {

    }

    void shutdown() {

    }

}


void vulkan_device_register(GPUDeviceAPI& gpu_device_api) {
    gpu_device_api.init = vk::init;
    gpu_device_api.shutdown = vk::shutdown;
    gpu_device_api.create_swapchain = vk::create_swapchain;
    gpu_device_api.destroy_swapchain = vk::destroy_swapchain;
}