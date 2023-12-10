#pragma once

#include "gpu_device.hpp"

struct GPUDeviceAPI {
    GPUResult       (*init)();
    GPUSwapchain    (*create_swapchain)(const SwapchainCreation& swapchain_creation, Window* window);
    void            (*destroy_swapchain)(const GPUSwapchain& swapchain);
    void            (*shutdown)();
};
