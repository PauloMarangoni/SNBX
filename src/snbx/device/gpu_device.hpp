#pragma once

#include "snbx/common.hpp"

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

SNBX_API void         gpu_device_init();
SNBX_API GPUSwapchain gpu_device_create_swapchain(const SwapChainCreation& swapchain_creation, Window* window);
SNBX_API void         gpu_device_destroy_swapchain(const GPUSwapchain& swapchain);
SNBX_API void         gpu_device_shutdown();
