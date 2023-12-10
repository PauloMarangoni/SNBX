#pragma once

#include "snbx/common.hpp"
#include "snbx/math/math_types.hpp"
#include "gpu_types.hpp"


struct Window;

SNBX_API GPUResult          gpu_device_init();
SNBX_API GPUSwapchain       gpu_device_create_swapchain(const SwapchainCreation& swapchain_creation, Window* window);
SNBX_API GPUPipelineState   gpu_device_create_graphics_pipeline(const GraphicsPipelineCreation& graphics_pipeline_creation);

SNBX_API void               gpu_device_begin_render_pass(const GPUCommands& gpu_commands, const BeginRenderPassInfo& begin_render_pass_info);
SNBX_API void               gpu_device_end_render_pass(const GPUCommands& gpu_commands);
SNBX_API void               gpu_device_set_viewport(const GPUCommands& cmd, const ViewportInfo& viewport_info);
SNBX_API void               gpu_device_set_scissor(const GPUCommands& cmd, const Rect& rect);

SNBX_API GPUCommands        gpu_device_begin_frame();
SNBX_API void               gpu_device_end_frame(const GPUSwapchain& swapchain);
SNBX_API void               gpu_device_destroy_swapchain(const GPUSwapchain& swapchain);
SNBX_API void               gpu_device_wait();
SNBX_API void               gpu_device_shutdown();
