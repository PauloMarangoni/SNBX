#pragma once

#include "gpu_device.hpp"

struct GPUDeviceAPI {
    GPUResult       (*init)();
    GPUSwapchain    (*create_swapchain)(const SwapchainCreation& swapchain_creation, Window* window);

    void            (*begin_render_pass)(const GPUCommands& cmd, const BeginRenderPassInfo& begin_render_pass_info);
    void            (*end_render_pass)(const GPUCommands& cmd);
    void            (*set_viewport)(const GPUCommands& cmd, const ViewportInfo& viewport_info);
    void            (*set_scissor)(const GPUCommands& cmd, const Rect& rect);

    GPUCommands     (*begin_frame)();
    void            (*end_frame)(const GPUSwapchain& swapchain);
    void            (*destroy_swapchain)(const GPUSwapchain& swapchain);
    void            (*wait)();
    void            (*shutdown)();
};
