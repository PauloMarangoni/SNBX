#pragma once

#include "gpu_device.hpp"

struct GPUDeviceAPI {
    void            (*init)();
    GPUSwapchain    (*create_swapchain)(const SwapChainCreation& swapchain_creation, Window* window);
    void            (*destroy_swapchain)(const GPUSwapchain& swapchain);
    void            (*shutdown)();
};
