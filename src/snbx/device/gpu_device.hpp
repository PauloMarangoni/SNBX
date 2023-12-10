#pragma once

#include "snbx/common.hpp"
#include "gpu_types.hpp"


struct Window;

SNBX_API GPUResult    gpu_device_init();
SNBX_API GPUSwapchain gpu_device_create_swapchain(const SwapchainCreation& swapchain_creation, Window* window);
SNBX_API void         gpu_device_destroy_swapchain(const GPUSwapchain& swapchain);
SNBX_API void         gpu_device_shutdown();
