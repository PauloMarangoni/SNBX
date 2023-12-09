#pragma once

#define GPU_HANDLER(StructName) struct StructName {         \
void* handler;                                              \
operator bool() const {return handler != nullptr; }		    \
bool operator==(const StructName& b) const { return this->handler == b.handler; } \
bool operator!=(const StructName& b) const { return this->handler != b.handler; } \
}

GPU_HANDLER(GPUSwapchain);

struct SwapChainCreation {
    bool vsync = true;
};

struct Window;

void                gpu_device_init();
GPUSwapchain        gpu_device_create_swapchain(const SwapChainCreation& swapchain_creation, Window* window);
void                gpu_device_destroy_swapchain(const GPUSwapchain& swapchain);
void                gpu_device_shutdown();
